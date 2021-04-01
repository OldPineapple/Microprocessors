#include "kalmanFilter.h"

void kalman_update (kalman_state* self, float* measurement)
{
	self->p = self->p + self->q;
	self->k = self->p / (self->p + self->r);
	self->x = self->x + self->k * (*measurement - self->x);
	self->p = (1 - self->k) * self->p;
}

int update (float* InputArray, float* OutputArray, kalman_state* kstate, int Length) {
	for (int i = 0; i < Length; i++) {
		kalman_update(kstate, &InputArray[i]);
		OutputArray[i] = kstate->x;
	}
	return 0;
}
