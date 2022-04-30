#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include <stdlib.h>
#include <math.h>
#define Word16 short
#define Word32 long
#define Float32 float
#define Float64 double
#define W_ord 2000
#define H1_TRUE_ORD 18
#define Sec_ord 12
#define STEP 0.15f


//主18次12
const double A[H1_TRUE_ORD] = {
	0.06438408701907, -0.02519421477958,  0.04155370049251, -0.04324502214555,
	0.06668137412364, -0.07960309326041,   0.1270708646786,   -0.204569835881,
	 0.6403536044635,   0.6403536044635,   -0.204569835881,   0.1270708646786,
   -0.07960309326041,  0.06668137412364, -0.04324502214555,  0.04155370049251,
   -0.02519421477958,  0.06438408701907
};
const double C[Sec_ord] = {//实际的次级通路
	-0.1026896085809, 0.05829573485312, -0.08077890047296, 0.1199333831213,
		-0.2076756622808, 0.6350878447541, 0.6350878447541, -0.2076756622808,
		0.1199333831213, -0.08077890047296, 0.05829573485312, -0.1026896085809
};
const double C1[Sec_ord] = {//预测的次级通路
	-0.1026896085809, 0.05829573485312, -0.08077890047296, 0.1199333831213,
		-0.2076756622808, 0.6350878447541, 0.6350878447541, -0.2076756622808,
		0.1199333831213, -0.08077890047296, 0.05829573485312, -0.1026896085809
};

int main()
{
	Word32 i, j;
	Word32 point;
	Word16 vi, y_err, d_n1, vi1;
	Float64 d_n, y_n, x_yn2, y1_n, v_x1;
	Float64 x_dn[H1_TRUE_ORD] = { 0 };
	Float64 w[W_ord] = { 0 };
	Float64 x_yn[W_ord] = { 0 };
	Float64 x1_n[W_ord] = { 0 };
	Float64 y_y1n[Sec_ord] = { 0 };

	FILE* f_x, * f_y, * f_x1;

	//FILE* f_noise, * f_out1;
	//fopen_s(&f_noise, "babble.pcm", "rb");
	//fopen_s(&f_out1, "out1.pcm", "wb");

	f_x = fopen("whitenoise.pcm", "rb");//babble_rs
	if (f_x == NULL) { printf("Cann't open the nosie file!!!\n");  exit(0); }
	//f_x1 = fopen("300_2k.pcm", "rb");
	//if (f_x1 == NULL) { printf("Cann't open the nosie file!!!\n");  exit(0); }
	f_y = fopen("out.pcm", "wb");
	if (f_y == NULL) { printf("Cann't open the out file!!!\n");  exit(0); }
	//fseek(f_music, 44L, SEEK_SET);

	//if ((f_x = fopen("whitenoise.txt", "r")) == NULL)//whitenoise  sin500
	//{
	//	printf("Fail to open file  f_x\n");
	//	exit(0);
	//}
	//if ((f_y = fopen("out.txt", "w")) == NULL)
	//{
	//	printf("Fail to open file f_y\n");
	//	exit(0);
	//}



	for (point = 0; point < 1000000; point++) {

		if (fread(&vi, sizeof(Word16), 1L, f_x) != 1L)  break;
		//if (fread(&vi1, sizeof(Word16), 1L, f_x1) != 1L)  break;

		//if (fscanf(f_x, "%ld", &vi) == EOF) break;

		if ((point % 8000) == 0)  printf("%ld points have been processed...\n", point);
		//求d(n)
		//相当于FIR滤波器，右移一位，新的赋值到x[0]
		for (i = H1_TRUE_ORD - 1; i > 0; i--) x_dn[i] = x_dn[i - 1];
		x_dn[0] = vi;
		for (d_n = i = 0; i < H1_TRUE_ORD; i++) d_n = d_n + A[i] * x_dn[i];

		//求y(n)
		for (i = W_ord - 1; i > 0; i--) x_yn[i] = x_yn[i - 1];
		x_yn[0] = vi;
		for (y_n = i = 0; i < W_ord; i++) y_n = y_n + w[i] * x_yn[i];
		//求y1(n)
		for (i = Sec_ord - 1; i > 0; i--) y_y1n[i] = y_y1n[i - 1];
		y_y1n[0] = y_n;
		for (y1_n = i = 0; i < Sec_ord; i++) y1_n = y1_n + C[i] * y_y1n[i];

		//求x1(n)
		for (v_x1 = i = 0; i < Sec_ord; i++) v_x1 = v_x1 + C[i] * x_dn[i];
		for (i = W_ord - 1; i > 0; i--) x1_n[i] = x1_n[i - 1];//倒序
		x1_n[0] = v_x1;
		//x1_n[0] = vi1;

		//修正w(n)
		for (i = 0, x_yn2 = 0; i < W_ord; i++) x_yn2 += x1_n[i] * x1_n[i];

		y_err = (Word16)(d_n - y1_n);

		for (i = 0; i < W_ord; i++) w[i] = w[i] + STEP * x1_n[i] * y_err / (x_yn2 + 1);
		//x_yn2起到归一化作用，可以将步长控制在0-1之间 大一点就收敛快，小一点就收敛慢
		//fprintf(f_y, "%d\n", y_err);
		d_n1 = (Word16)(d_n);
		fwrite(&y_err, sizeof(Word16), 1L, f_y);
	}
	for (i = 0; i < W_ord; i++) printf("%.10lf\n", w[i]);
	fclose(f_x);
	fclose(f_y);
}
