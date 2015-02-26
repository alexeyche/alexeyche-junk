# $HOME/.bash_aliases for bash-3.0 (or later)


# ---------------
# Personal
# connect to remote host
alias t='ssh twalrus'
alias t2='ssh twalrus2'
alias g='ssh ghad'
alias t3='ssh twalrus3'
alias tns='ssh twalrus screen -S alexeyche_ssh -d -m'
alias ts='ssh -t twalrus screen -S alexeyche_ssh -r' 
alias t2ns='ssh twalrus2 screen -S alexeyche_ssh -d -m'
alias t2s='ssh -t twalrus2 screen -S alexeyche_ssh -r'
alias gns='ssh ghad screen -S alexeyche_ssh -d -m'
alias gs='ssh -t ghad screen -S alexeyche_ssh -r'
alias t3ns='ssh twalrus3 screen -S alexeyche_ssh -d -m'
alias t3s='ssh -t twalrus3 screen -S alexeyche_ssh -r'
alias t4='ssh twalrus4'
alias t5='ssh twalrus5'
alias t4ns='ssh twalrus4.yandex.ru screen -S alexeyche_ssh -d -m'
alias t4s='ssh -t twalrus4.yandex.ru screen -S alexeyche_ssh -r'
alias t5ns='ssh twalrus5.yandex.ru screen -S alexeyche_ssh -d -m'
alias t5s='ssh -t twalrus5.yandex.ru screen -S alexeyche_ssh -r'
alias rmrf='rm -rf ./*'
alias ya="/home/alexeyche/wd/arc/arcadia/devtools/ya/ya"
alias cb='if readlink -f . | grep -Eq "build$"; then rm -rf ./*; else echo "This folder doesnt look like build directory"; fi'
alias sl='screen -list'
alias sr='screen -r'
# ---------------
# Common
alias vim_ro='vim -R'
# safe mode
alias rm='rm -i'
alias cp='cp -i'
alias mv='mv -i'
# mkdir
alias mkdir='mkdir -p'
# scroller
alias less='less -r'
# delete
alias del='rm -rf'
# dir list
alias ls='ls -FG' 
alias ll='ls -l' 
alias la='ll -A'
alias l='ls -CF'
alias lh='ls -alh'
# deb
alias dch='dch --distributor=debian'
# others
alias vi='vim'
alias up='svn cleanup;svn up'
alias chfile='chmod a=r,u+w'
alias chdir='chmod a=rx,u+w'
alias lvim='vim -S "~/.lastVimSession"'
# change svn version
alias svn5=' change-svn-wc-format.py $PWD 1.5;svn cleanup'
alias svn4=' change-svn-wc-format.py $PWD 1.4;svn cleanup'


# ---------------
# Color ls
if [ -x /usr/bin/dircolors ]; then
    eval "`dircolors -b`"
    alias ls='ls --color=auto'
    alias grep='grep --color=auto'
fi
