#include <stdio.h>
#include <string.h>
#include <math.h>

#include "MVerb.h"

int main(int argc, const char* argv[]) {
float **input;
float **output;
input=new float*[2];
output=new float*[2];
input[0]=new float[10];
input[1]=new float[10];
output[0]=new float[10];
output[1]=new float[10];
for (int i =0;i<10;++i) {
input[0][i]=i*0.092;
input[1][i]=i*0.092;
}
MVerb<float> mv;
mv.process(input, output, 10);
for (int i =0;i<10;++i) {
printf("%f:%f  %f:%f\n",input[0][i],input[1][i],output[0][i],output[1][i]);
}

delete []input[0];
delete []input[1];
delete []input;
delete []output[0];
delete []output[1];
delete []output;
return 0;

}

