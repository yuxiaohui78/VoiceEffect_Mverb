#include <stdio.h>
#include <string.h>
#include <math.h>

#include "MVerb.h"

#include <stdio.h>
#include <stdlib.h>

extern "C"
{
typedef struct  WAV_HEADER
{
	char                RIFF[4];        
	int                 ChunkSize;     
	char                WAVE[4];       
	char                fmt[4];        
	int                 Subchunk1Size;                              
	short int           AudioFormat;  
	short int           NumOfChan;      
	int                 SamplesPerSec;  
	int                 bytesPerSec;    
	short int           blockAlign;    
	short int           bitsPerSample;  
	int                 Subchunk2Size; 
	char                Subchunk2ID[4];
}wav_hdr; 

int getFileSize(FILE *inFile); 

char *getContentDataBuf (int size, FILE *inFile){
	char *buf = (char*)malloc (size);
	if (buf == NULL){
		printf ("Allocate buffer error!\n");
		return NULL;
	}

	memset (buf, 0, size);

	fread(buf, size, 1, inFile);
	return buf;
}

int getFileSize(FILE *inFile)
{
	int fileSize = 0;
	fseek(inFile,0,SEEK_END);
	fileSize=ftell(inFile);
	fseek(inFile,0,SEEK_SET);
	return fileSize;
}

static MVerb<float>* getMVerb(bool del) {
	static MVerb<float>* mv = NULL;
	if (del) {
		if (mv != NULL) {

			delete(mv);
		}
		mv = NULL;
		return NULL;
	} else {
		if (mv == NULL) {
			mv = new MVerb<float>;

		}
		return mv;
	}
}

int MY_SAMPLING_RATE = 0;

static void writeWavFile(const char * data, int size, const char * fileName)
{
	FILE * fp=fopen(fileName,"wb");

	// write header
	fwrite ("RIFF" , 1 , 4, fp); // 
	int totalSize = size+36;
	fwrite(&totalSize,1,sizeof(int),fp);
	fwrite ("WAVE" , 1 , 4, fp);
	fwrite ("fmt " , 1 , 4, fp);
	int sub_channel = 16; //Sub-chunk size, 16 for PCM
	fwrite(&sub_channel,1,sizeof(int),fp);  

	short audioFormat =1 ;//AudioFormat, 1 for PCM
	fwrite(&audioFormat,1,sizeof(short),fp);

	short channel = 1; //Number of channels, 1 for mono, 2 for stereo
	fwrite(&channel,1,sizeof(short),fp);

	int sRate = MY_SAMPLING_RATE; // Sample rate
	fwrite(&sRate,1,sizeof(int),fp);

	int byteRate = MY_SAMPLING_RATE*2 ;//Byte rate, SampleRate*NumberOfChannels*BitsPerSample/8
	fwrite(&byteRate,1,sizeof(int),fp);

	short block = 2 ;//Block align, NumberOfChannels*BitsPerSample/8
	fwrite(&block,1,sizeof(short),fp);

	short bSample = 16; //bits per sample
	fwrite(&bSample,1,sizeof(short),fp);

	fwrite ("data" , 1 , 4, fp);

	fwrite(&size,1,sizeof(int),fp);

	fwrite(data,1,size,fp);

	fclose(fp);
}

float getMaximumAmplitude (float **output, int len){
	float max = output[0][0];
	float min = output[0][0];
	float maxAmplitude = 32768.0;
	
	for (int j=0; j<len; ++j) {
		if (max < output[0][j]) max = output[0][j];
		if (min > output[0][j]) min = output[0][j];
	}

	printf ("max=%f, min=%f\n", max, min);			

	if (max < fabs(min)) max = fabs(min);

	if (max >= 1.0) 
	{
		maxAmplitude = 32768.0 / (max + 0.01);
	}
	return maxAmplitude;
}

void reverb_wave (char *effect, const char *inBuf, int inSize){
	//MVerb<float>* mv = new MVerb<float>;
	MVerb<float>* mv = getMVerb(false);
	if (!strcmp(effect, "STADIUM")) {
		mv->setParameter(MVerb<float>::DAMPINGFREQ, 0.);
		mv->setParameter(MVerb<float>::DENSITY, 0.5);
		mv->setParameter(MVerb<float>::BANDWIDTHFREQ, 1.);
		mv->setParameter(MVerb<float>::DECAY, 0.5);
		mv->setParameter(MVerb<float>::PREDELAY, 0.);
		mv->setParameter(MVerb<float>::SIZE, 1.);
		mv->setParameter(MVerb<float>::GAIN, 1.);
		mv->setParameter(MVerb<float>::MIX, 0.35);
		mv->setParameter(MVerb<float>::EARLYMIX, 0.75);
	}
	if (!strcmp(effect, "SUBTLE")) {
		mv->setParameter(MVerb<float>::DAMPINGFREQ, 0.);
		mv->setParameter(MVerb<float>::DENSITY, 0.5);
		mv->setParameter(MVerb<float>::BANDWIDTHFREQ, 1.);
		mv->setParameter(MVerb<float>::DECAY, 0.5);
		mv->setParameter(MVerb<float>::PREDELAY, 0.);
		mv->setParameter(MVerb<float>::SIZE, 0.5);
		mv->setParameter(MVerb<float>::GAIN, 1.);
		mv->setParameter(MVerb<float>::MIX, 0.15);
		mv->setParameter(MVerb<float>::EARLYMIX, 0.75);
	}
	if (!strcmp(effect, "CUPBOARD")) {
		mv->setParameter(MVerb<float>::DAMPINGFREQ, 0.);
		mv->setParameter(MVerb<float>::DENSITY, 0.5);
		mv->setParameter(MVerb<float>::BANDWIDTHFREQ, 1.);
		mv->setParameter(MVerb<float>::DECAY, 0.5);
		mv->setParameter(MVerb<float>::PREDELAY, 0.);
		mv->setParameter(MVerb<float>::SIZE, 0.25);
		mv->setParameter(MVerb<float>::GAIN, 1.);
		mv->setParameter(MVerb<float>::MIX, 0.35);
		mv->setParameter(MVerb<float>::EARLYMIX, 0.75);
	}
	if (!strcmp(effect, "DARK")) {
		mv->setParameter(MVerb<float>::DAMPINGFREQ, 0.9);
		mv->setParameter(MVerb<float>::DENSITY, 0.5);
		mv->setParameter(MVerb<float>::BANDWIDTHFREQ, 0.1);
		mv->setParameter(MVerb<float>::DECAY, 0.5);
		mv->setParameter(MVerb<float>::PREDELAY, 0.);
		mv->setParameter(MVerb<float>::SIZE, 0.5);
		mv->setParameter(MVerb<float>::GAIN, 1.);
		mv->setParameter(MVerb<float>::MIX, 0.5);
		mv->setParameter(MVerb<float>::EARLYMIX, 0.75);
	}
	if (!strcmp(effect, "HALVES")) {
		mv->setParameter(MVerb<float>::DAMPINGFREQ, 0.5);
		mv->setParameter(MVerb<float>::DENSITY, 0.5);
		mv->setParameter(MVerb<float>::BANDWIDTHFREQ, 0.5);
		mv->setParameter(MVerb<float>::DECAY, 0.5);
		mv->setParameter(MVerb<float>::PREDELAY, 0.5);
		mv->setParameter(MVerb<float>::SIZE, 0.75);
		mv->setParameter(MVerb<float>::GAIN, 1.);
		mv->setParameter(MVerb<float>::MIX, 0.5);
		mv->setParameter(MVerb<float>::EARLYMIX, 0.5);	
	}

	float **input;
	float **output;
	input=new float*[2];
	output=new float*[2];

	const int frameSize = 44100*3; 
	// add reverb
	short * inShort = (short *) inBuf;

	int fSize = inSize / sizeof(short);

	input[0] = (float*)malloc(frameSize*sizeof(float));	
	output[0] = (float*)malloc(frameSize*sizeof(float));
	if (input[0] == NULL || output[0] == NULL) {
		printf ("&&&&&&&&&&&&&&&&& effect out of memory &&&&&&&&&&&&&&&\n");
		return ;
	}

	input[1] = input[0];
	output[1] = output[0];

	int k = 0;
	int totalFrame = fSize/frameSize + 1;

	printf ("frameSize=%d\n", frameSize);
	printf ("totalFrame=%d\n", totalFrame);

	if (strcmp(effect, "NONE")) {
		for (int k=0; k<totalFrame; ++k) {	
			int i=0;
			while (i<frameSize && (k*frameSize+i)<fSize) {
				input[0][i]=inShort[k*frameSize+i]/32768.0* 0.9;				
				i++;
			}

			mv->process(input, output, i);
			

			float enlargedMaxAmplitude = getMaximumAmplitude (output, i);
			
			// TODO" normalize by max
			for (int j=0; j<i; ++j) {

				inShort[k*frameSize+j]=(short)((output[0][j])*enlargedMaxAmplitude);  			  		
			}	
		}
	}


	writeWavFile(inBuf, fSize*sizeof(short), "reverbed_wave.wav");
	//writeWavFile(inBuf+frameSize*pos, frameSize*sizeof(short), "reverbed_wave.wav");
	printf("finished effect, write %d successful\n", fSize);
}



int main(int argc,char *argv[])
{
	//check startup conditions
	if(argc >= 2); //we have enough arguments -- continue
	else { printf("\nUSAGE: program requires a filename as an argument -- please try again\n"); exit(0);}

	wav_hdr wavHeader;
	FILE *wavFile;
	char * dataBuf = NULL;

	int headerSize = sizeof(wav_hdr),filelength = 0;
	int contentDataSize = 0;

	wavFile = fopen(argv[1],"r");
	if(wavFile == NULL)
	{
		printf("Unable to open wave file\n");
		exit(EXIT_FAILURE);
	}
	fread(&wavHeader,headerSize,1,wavFile);
	filelength = getFileSize(wavFile);
	contentDataSize = filelength - headerSize;

	fseek(wavFile, headerSize, SEEK_SET);
	dataBuf = getContentDataBuf (contentDataSize, wavFile);

	fclose(wavFile);

	MY_SAMPLING_RATE = wavHeader.SamplesPerSec;

	printf("\nWav file header information:\n");
	printf("Filesize\t\t\t%d bytes\n",filelength);
	printf("RIFF header\t\t\t%c%c%c%c\n",wavHeader.RIFF[0],wavHeader.RIFF[1],wavHeader.RIFF[2],wavHeader.RIFF[3]);
	printf("WAVE     header\t\t\t%c%c%c%c\n",wavHeader.WAVE[0],wavHeader.WAVE[1],wavHeader.WAVE[2],wavHeader.WAVE[3]);
	printf("Subchunk1ID\t\t\t%c%c%c%c\n",wavHeader.fmt[0],wavHeader.fmt[1],wavHeader.fmt[2],wavHeader.fmt[3]);
	printf("Chunk Size (based on bits used)\t%d\n",wavHeader.ChunkSize);
	printf("Subchunk1Size\t\t\t%d\n",wavHeader.Subchunk1Size);
	printf("Sampling Rate\t\t\t%d\n",wavHeader.SamplesPerSec); //Sampling frequency of the wav file
	printf("Bits Per Sample\t\t\t%d\n",wavHeader.bitsPerSample); //Number of bits used per sample
	printf("AudioFormat\t\t\t%d\n",wavHeader.AudioFormat);
	printf("Number of channels\t\t%d\n",wavHeader.NumOfChan);     //Number of channels (mono=1/sterio=2)
	printf("Byte Rate\t\t\t%d\n",wavHeader.bytesPerSec);   //Number of bytes per second
	printf("Subchunk2ID\t\t\t%c%c%c%c\n",wavHeader.Subchunk2ID[0],wavHeader.Subchunk2ID[1],wavHeader.Subchunk2ID[2],wavHeader.Subchunk2ID[3]);
	printf("Subchunk2Size\t\t\t%d\n",wavHeader.Subchunk2Size);
	printf("\n");

	if (NULL == dataBuf){

		return 0;
	}

	printf("reverb wave...\n");
	char *ep[] = {"STADIUM","SUBTLE","CUPBOARD", "DARK","HALVES"};
	reverb_wave (ep[0], dataBuf, contentDataSize);

	return 0;
}

};