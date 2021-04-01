#ifndef CAL_H
#define CAL_H

	extern void Subarray (float data1[], float data2[], float result[], int size);
	extern void Average (float input[], float* output, int size);
	extern void SD (float input[], float* mean, float* output, int size);
	extern void Correlate(float Signal[], int SignalLen, float Kernel[], int KernelLen, float Result[]);
	extern void Convolve(float Signal[], int SignalLen, float Kernel[], int KernelLen, float Result[]);


#endif
