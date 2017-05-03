#include <iostream>
#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


__global__ void run(unsigned char *source, int width, int height, unsigned char *destination) {
    int x = (blockIdx.x * blockDim.x) + threadIdx.x;
    int y = (blockIdx.y * blockDim.y) + threadIdx.y;

    float sum = 0.0;

    // int   pWidth = kWidth/2;
    // int   pHeight = kHeight/2;

    // // Only execute for valid pixels
    // if(x >= pWidth+paddingX &&
    //    y >= pHeight+paddingY &&
    //    x < (blockDim.x * gridDim.x)-pWidth-paddingX &&
    //    y < (blockDim.y * gridDim.y)-pHeight-paddingY)
    // {
    //     for(int j = -pHeight; j <= pHeight; j++)
    //     {
    //         for(int i = -pWidth; i <= pWidth; i++)
    //         {
    //             // Sample the weight for this location
    //             int ki = (i+pWidth);
    //             int kj = (j+pHeight);
    //             float w  = convolutionKernelStore[(kj * kWidth) + ki + kOffset];

        
    //             sum += w * float(source[((y+j) * width) + (x+i)]);
    //         }
    //     }
    // }
    
    destination[(y * width) + x] = (unsigned char) sum;
}

// create an image buffer.  return host ptr, pass out device pointer through pointer to pointer
void* createImageBuffer(unsigned int bytes, void **devicePtr)
{
    void *ptr = NULL;
    cudaSetDeviceFlags(cudaDeviceMapHost);
    cudaHostAlloc(&ptr, bytes, cudaHostAllocMapped);
    cudaHostGetDevicePointer(devicePtr, ptr, 0);
    return ptr;
}





int main (int argc, char** argv)
{
    // Open a webcamera
    cv::VideoCapture camera(0);
    cv::Mat          frame;
    if(!camera.isOpened()) 
        return -1;
    
    // Create the capture windows
    cv::namedWindow("Source");
    cv::namedWindow("Greyscale");
    cv::namedWindow("Dest");
    
    // Create the cuda event timers 
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    camera >> frame;
    
    unsigned char *sourceDevice;
    cv::Mat source  (frame.size(), CV_8U, createImageBuffer(frame.size().width * frame.size().height, (void**)(&sourceDevice)));
    
    size_t ncHeight = frame.size().height/10;
    size_t ncWidth = frame.size().width/10;
    size_t ncNum = ncWidth * ncHeight;
    cv::Size ncSize(ncWidth, ncHeight);

    float *weightsDevice;
    cv::Mat weights(ncSize, CV_32F, createImageBuffer(sizeof(float) * ncNum, (void**)&weightsDevice));
    
    
    unsigned char *destDevice;
    cv::Mat dest(frame.size(), CV_8U, createImageBuffer(frame.size().width * frame.size().height, (void**)(&destDevice)));
    
    while(1) {
        
        camera >> frame;
        cv::cvtColor(frame, source, CV_BGR2GRAY);
        
        cudaEventRecord(start);
        {
            dim3 cblocks (ncHeight / 4, ncWidth / 4);
            dim3 cthreads(4, 4);

            
            run<<<cblocks,cthreads>>>(sourceDevice, frame.size().width, frame.size().height, destDevice);
            
            cudaThreadSynchronize();
        }
        cudaEventRecord(stop);
        
        float ms = 0.0f;
        cudaEventSynchronize(stop);
        cudaEventElapsedTime(&ms, start, stop);
        std::cout << "Elapsed GPU time: " << ms << " milliseconds" << std::endl;

        // Show the results
        cv::imshow("Source", frame);
        cv::imshow("Greyscale", source);
        cv::imshow("Dest", dest);
        
        // Spin
        if(cv::waitKey(1) == 27) break;
    }
    
    // Exit
    cudaFreeHost(source.data);
    cudaFreeHost(dest.data);
    cudaFree(weights.data);    
    return 0;
}
