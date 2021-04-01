.syntax unified
.global kalman

kalman:
	PUSH {LR} // put the return address onto the stack
	VPUSH {S0-S6} // preserve the original value from S0 to S6
	VLDR.F32 S0, [R0] // floating point register S0 stores q
	VLDR.F32 S1, [R0, #4] // floating point register S1 stores r
	VLDR.F32 S2, [R0, #8] // floating point register S2 stores x
	VLDR.F32 S3, [R0, #12] // floating point register S3 stores p
	VLDR.F32 S4, [R0, #16] // floating point register S4 stores k
	VLDR.F32 S5, [R1] //floating point register S5 stores measurement

update:
	// first line in update
	VADD.F32 S3, S3, S0 // p = p + q
	// second line in update
	VADD.F32 S6, S3, S1 // S6 = p + r
	VDIV.F32 S4, S3, S6 // k = p / S6
	// third line in update
	VSUB.F32 S6, S5, S2 // S6 = measurement - x
	VMLA.F32 S2, S4, S6// x = x + k * S6
	// fourth line in update
	VMUL.F32 S6, S4, S3 // S6 = k * p
	VSUB.F32 S3, S3, S6 // p = p - S6

done:
	VSTR S0, [R0] // store q back
	VSTR S1, [R0, #4] // store r back
	VSTR S2, [R0, #8] // store x back
	VSTR S3, [R0, #12] // store p back
	VSTR S4, [R0, #16] // store k back
	VPOP {S0-S6} // restore original value from S0 to S6
	POP {PC} // fetch the return address off the stack and put it into PC

.end
