#pragma once

#include "http.h"
#include "response_builder.h"

#include <dnn/util/log/log.h>
#include <dnn/util/string.h>
#include <dnn/util/optional.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>

#include <future>
#include <regex>
#include <deque>

namespace NDnn {


	class TServer {
	public:
		using TRequestCallback = std::function<void(const THttpRequest&, TResponseBuilder&)>;

	private:
		struct TCallbackPath {
			TString Path;
			TString MatchingString;
			bool RegExp = false;

			std::deque<TString> Keywords;
			TRequestCallback Callback;
		};

		static constexpr ui32 ReceiveChunkSize = 1024;
		static constexpr ui32 SendChunkSize = 1024;

		static void* GetInAddr(struct sockaddr *sa)
		{
		    if (sa->sa_family == AF_INET) {
		        return &(((struct sockaddr_in*)sa)->sin_addr);
		    }

		    return &(((struct sockaddr_in6*)sa)->sin6_addr);
		}

		static int SendAll(int s, const char *buf, int *len) {
		    int total = 0;        // how many bytes we've sent
		    int bytesleft = *len; // how many we have left to send
		    int n;

		    while(total < *len) {
		        n = send(s, buf+total, bytesleft, 0);
		        if (n == -1) { break; }
		        total += n;
		        bytesleft -= n;
		    }

		    *len = total; // return number actually sent here

		    return n==-1?-1:0; // return -1 on failure, 0 on success
		}

		static const auto DefaultMaxConnections = 10;
	public:
		TServer(ui32 port, ui32 max_connections = DefaultMaxConnections, bool debugMode = false)
			: DebugMode(debugMode)
			, Port(port)
			, MaxConnections(max_connections)
		{
		}
		
		void Listen() {
			int status;
			struct addrinfo hints;

			memset(&hints, 0, sizeof hints); // make sure the struct is empty
			hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
			hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
			hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

			std::stringstream pss;
			pss << Port;
			std::string ps = pss.str();

			struct addrinfo *servinfo;
			if ((status = getaddrinfo(NULL, ps.c_str(), &hints, &servinfo)) != 0) {
			    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
			    exit(1);
			}

			for(struct addrinfo* p = servinfo; p != NULL; p = p->ai_next) {
		        if ((SocketNum = socket(p->ai_family, p->ai_socktype,
		                p->ai_protocol)) == -1) {
		            perror("server: socket");
		            continue;
		        }

				int yes=1;
		        if (setsockopt(SocketNum, SOL_SOCKET, SO_REUSEADDR, &yes,
		                sizeof(int)) == -1) {
		            perror("setsockopt");
		            exit(1);
		        }

		        if (bind(SocketNum, p->ai_addr, p->ai_addrlen) == -1) {
		            close(SocketNum);
		            perror("server: bind");
		            continue;
		        }

		        break;
		    }
			freeaddrinfo(servinfo);

			ENSURE(
				listen(SocketNum, MaxConnections) >= 0,
				"Failed to listen"
			);
		}
		
		const ui32& GetPort() const {
			return Port;
		}

		void SetPort(ui32 port) {
			Port = port;
		}

		TServer& AddCallback(TString method, TString path, TRequestCallback cb) {
			auto pathsPtr = Callbacks.find(method);
			if(pathsPtr == Callbacks.end()) {
				pathsPtr = Callbacks.insert(MakePair(method, std::vector<TCallbackPath>())).first;
			}
			TCallbackPath cbPath;
			std::regex keyRe("\\{([^\\}]+)\\}");
			std::sregex_iterator next(path.begin(), path.end(), keyRe);
			std::sregex_iterator end;
			while (next != end) {
			    std::smatch match = *next;
			    L_DEBUG << "Got keyword in callback: " << match[1];
			    cbPath.Keywords.push_back(match[1]);
			    next++;
			}
			if (cbPath.Keywords.size()>0) {
				cbPath.MatchingString = NStr::TStringBuilder() << "^" << std::regex_replace(path, keyRe, "([-.a-zA-Z0-9]+)") << "$";
				cbPath.RegExp = true;
				L_DEBUG << "Callback path regexp: " << cbPath.MatchingString;
			} else {
				cbPath.MatchingString = path;
				cbPath.RegExp = false;
				L_DEBUG << "Callback path simple match: " << cbPath.MatchingString;
			}
			cbPath.Callback = cb;
			pathsPtr->second.push_back(cbPath);
			return *this;
		}

		TServer& AddCallback(TString method, TRequestCallback cb) {
			auto res = DefaultCallbacks.insert(MakePair(method, cb));
			ENSURE(res.second, "Found duplicates for default callback for method " << method);
			return *this;
		}

		void MainLoop() {
			Listen();
			while (true) {
				struct sockaddr_storage their_addr;
				socklen_t addr_size = sizeof(their_addr);

				int new_fd = accept(SocketNum, (struct sockaddr *)&their_addr, &addr_size);
				ENSURE(new_fd >= 0, "Failed to accept to socket");

				char s[INET6_ADDRSTRLEN];
				inet_ntop(their_addr.ss_family,
            		GetInAddr((struct sockaddr *)&their_addr),
            		s, sizeof s);

				if (DebugMode) {
					Receive(new_fd);
        			close(new_fd);
				} else {
		            std::thread(
		            	[new_fd, this]() {
		            		Receive(new_fd);
		            		close(new_fd);
		            	}
		            ).detach();
				}
			}
		}

		void Receive(int socket) {
			TDeque<char> bytes;
			ui32 bytesReceived = 0;
			while (true) {
				TVector<char> chunk(ReceiveChunkSize);

				int chunkReceived = recv(socket, &chunk[0], ReceiveChunkSize, 0);
				if (chunkReceived < 0) {
					perror("Some errors while reading recv socket");
					exit(0);
				}

				if (chunkReceived == 0) {
					break;
				}

				bytesReceived += chunkReceived;
				
				bytes.insert(bytes.end(), chunk.begin(), chunk.begin() + chunkReceived);
				
				if (chunkReceived < ReceiveChunkSize) {
					break;
				}
			}

			if (bytesReceived == 0) {
				L_DEBUG << "Received zero bytes from socket. Ignoring";
				return;
			}
			
			THttpRequest req = ParseHttpRequest(std::move(bytes));

			TOptional<TRequestCallback> cb;

			auto methodCbPtr = Callbacks.find(req.Method);
			if (methodCbPtr != Callbacks.end()) {
				for (const auto& cbPath: methodCbPtr->second) {
					if (!cbPath.RegExp  && (cbPath.MatchingString == req.Path)) {
						cb = cbPath.Callback;
						break;
					} else
					if (cbPath.RegExp) {
						std::map<TString, TString> keywordMap;
						std::deque<TString> keys = cbPath.Keywords;

						std::regex cbPathRe(cbPath.MatchingString);
						std::sregex_iterator next(req.Path.begin(), req.Path.end(), cbPathRe);
						std::sregex_iterator end;
						while (next != end) {
						    std::smatch match = *next;
						    ENSURE(keys.size() > 0, "Got extra keys in path: " << req.Path << ", key: " << match[1]);
						    keywordMap.insert(MakePair(keys.front(), match[1]));
						    L_DEBUG << "Got keywords in path " << keys.front() << " -> " << match[1];
						    keys.pop_front();
						    next++;
						}

						if (keywordMap.size()>0) {
							cb = cbPath.Callback;
							req.KeywordsMap = keywordMap;
							break;
						}
					}
				}
			}
			if (!cb) {
				auto defCbPtr = DefaultCallbacks.find(req.Method);
				ENSURE(defCbPtr != DefaultCallbacks.end(), "Can't find appropriate callback for method " << req.Method);
				cb = defCbPtr->second;
			}

			TResponseBuilder respBuilder(req);
			THttpResponse resp;

			try {
				(*cb)(req, respBuilder);
				resp = respBuilder
				    .FormResponse();
			} catch (const TDnnFileNotFound& e) {
				resp = respBuilder
					.StaticFile("not_found.html")
					.NotFound()
				    .FormResponse();
			    if (DebugMode) throw;
			} catch (const TDnnElementNotFound& e) {
				resp = respBuilder
					.StaticFile("not_found.html")
					.NotFound()
				    .FormResponse();
			    if (DebugMode) throw;
			} catch (const TDnnLogicError& e) {
				resp = respBuilder
					.Body(e.what())
					.BadRequest()
					.FormResponse();
				if (DebugMode) throw;
			} catch (const TDnnNotAvailable& e) {
				resp = respBuilder
					.Body(e.what())
					.ServiceUnavailable()
					.FormResponse();
				if (DebugMode) throw;
			} catch (const std::exception& e) {
				L_DEBUG << "Internal error: " << e.what();
				resp = respBuilder
					.Body(e.what())
					.InternalError()
				    .FormResponse();
				if (DebugMode) throw;
			}

			L_DEBUG << req.Method << " " << req.Path << " -> " << resp.Code << " " << resp.Status;

			std::ostringstream respStr;
			respStr << resp;
			TString respStrInst = respStr.str();
			const char *respStrArray = respStrInst.c_str();
			int len = respStrInst.size();
			ENSURE(SendAll(socket, respStrArray, &len) >= 0, "Failed to send data");
		}

		~TServer() {
			close(SocketNum);
		}

	private:
		bool DebugMode;
		ui32 Port;
		ui32 MaxConnections;

		int SocketNum;
		std::map<TString, TRequestCallback> DefaultCallbacks;
		std::map<TString, std::vector<TCallbackPath>> Callbacks;
	};

} // namespace NDnn