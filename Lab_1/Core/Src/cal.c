#include "cal.h"
#include <math.h>
#include <stdio.h>
#include <complex.h>

void Subarray (float data1[], float data2[], float result[], int size)
{
	for(int i = 0; i < size; i++)
		result[i] = data1[i] - data2[i];
}

void Average (float input[], float* output, int size)
{
	float sum = 0;

	for(int i = 0; i < size; i++)
		sum = sum + input[i];

	*output = sum / size;
}

void SD (float input[], float* mean, float* output, int size)
{
	float SD = 0.0;

	for(int i = 0; i < size; i++)
		SD += powf(input[i] - *mean, 2);

	*output = sqrtf(SD/size);
}

void Correlate(float Signal[], int SignalLen, float Kernel[], int KernelLen, float Result[])
{
	int m = 0;

	for (int k = 0; k < SignalLen + KernelLen - 1; k++)
	{
		Result[k] = 0;
		for (int l = 0; l < SignalLen; l++)
        {

        	m = l- k + SignalLen - 1;
        	if(m > KernelLen - 1 || m < 0);
        	else
        		Result[k] = Result[k] + Signal[l] * conj(Kernel[m]);
        }
	}
}

void Convolve(float Signal[], int SignalLen, float Kernel[], int KernelLen, float Result[])
{
  int n = 0;

  for (n = 0; n < SignalLen + KernelLen - 1; n++)
  {
    int kmin, kmax, k;

    Result[n] = 0;

    kmin = (n >= KernelLen - 1) ? n - (KernelLen - 1) : 0;
    kmax = (n < SignalLen - 1) ? n : SignalLen - 1;

    for (k = kmin; k <= kmax; k++)
    	Result[n] += Signal[k] * conj(Kernel[n - k]);
  }
}
