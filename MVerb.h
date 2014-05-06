//	Copyright (c) 2010 Martin Eastwood
//  This code is distributed under the terms of the GNU General Public License

//  MVerb is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  at your option) any later version.
//
//  MVerb is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this MVerb.  If not, see <http://www.gnu.org/licenses/>.

#ifndef EMVERB_H
#define EMVERB_H

//forward declaration
template<typename T, int maxLength> class Allpass;
template<typename T, int maxLength> class StaticAllpassFourTap;
template<typename T, int maxLength> class StaticDelayLine;
template<typename T, int maxLength> class StaticDelayLineFourTap;
template<typename T, int maxLength> class StaticDelayLineEightTap;
template<typename T, int OverSampleCount> class StateVariable;


template<typename T>
class MVerb
{
private:
    Allpass<T, 96000> allpass0, allpass1, allpass2, allpass3;
    StaticAllpassFourTap<T, 96000> allpassFourTap0, allpassFourTap1, allpassFourTap2, allpassFourTap3;
    StateVariable<T,4> bandwidthFilter0, bandwidthFilter1;
    StateVariable<T,4> damping0, damping1;
    StaticDelayLine<T, 96000> predelay;
    StaticDelayLineFourTap<T, 96000> staticDelayLine0, staticDelayLine1, staticDelayLine2, staticDelayLine3;
    StaticDelayLineEightTap<T, 96000> earlyReflectionsDelayLine0, earlyReflectionsDelayLine1;
    T SampleRate, DampingFreq, Density1, Density2, BandwidthFreq, PreDelayTime, Decay, Gain, Mix, EarlyMix, Size;
    T MixSmooth, EarlyLateSmooth, BandwidthSmooth, DampingSmooth, PredelaySmooth, SizeSmooth, DensitySmooth, DecaySmooth;
    T PreviousLeftTank, PreviousRightTank;
    int ControlRate, ControlRateCounter;

public:
    enum
		{
			DAMPINGFREQ=0,
			DENSITY,
			BANDWIDTHFREQ,
            DECAY,
            PREDELAY,
            SIZE,
            GAIN,
            MIX,
            EARLYMIX,
            NUM_PARAMS
		};

    MVerb(){
        DampingFreq = 18000.;
        BandwidthFreq = 18000.;
        SampleRate = 44100.;
        Decay = 0.5;
        Gain = 1.;
        Mix = 1.;
        Size = 1.;
        EarlyMix = 1.;
        PreviousLeftTank = 0.;
        PreviousRightTank = 0.;
        PreDelayTime = 100 * (SampleRate / 1000);
        MixSmooth = EarlyLateSmooth = BandwidthSmooth = DampingSmooth = PredelaySmooth = SizeSmooth = DecaySmooth = DensitySmooth = 0.;
        ControlRate = SampleRate / 1000;
        ControlRateCounter = 0;
        reset();
    }

    ~MVerb(){
        //nowt to do here
    }

    void process(T **inputs, T **outputs, int sampleFrames){
        T OneOverSampleFrames = 1. / sampleFrames;
        T MixDelta	= (Mix - MixSmooth) * OneOverSampleFrames;
        T EarlyLateDelta = (EarlyMix - EarlyLateSmooth) * OneOverSampleFrames;
        T BandwidthDelta = (((BandwidthFreq * 18400.) + 100.) - BandwidthSmooth) * OneOverSampleFrames;
        T DampingDelta = (((DampingFreq * 18400.) + 100.) - DampingSmooth) * OneOverSampleFrames;
        T PredelayDelta = ((PreDelayTime * 200 * (SampleRate / 1000)) - PredelaySmooth) * OneOverSampleFrames;
        T SizeDelta	= (Size - SizeSmooth) * OneOverSampleFrames;
        T DecayDelta = (((0.7995f * Decay) + 0.005) - DecaySmooth) * OneOverSampleFrames;
        T DensityDelta = (((0.7995f * Density1) + 0.005) - DensitySmooth) * OneOverSampleFrames;
        for(int i=0;i<sampleFrames;++i){        	
            T left = inputs[0][i];
            T right = inputs[1][i];
            MixSmooth += MixDelta;
            EarlyLateSmooth += EarlyLateDelta;
            BandwidthSmooth += BandwidthDelta;
            DampingSmooth += DampingDelta;
            PredelaySmooth += PredelayDelta;
            SizeSmooth += SizeDelta;
            DecaySmooth += DecayDelta;
            DensitySmooth += DensityDelta;
            if (ControlRateCounter >= ControlRate){
                ControlRateCounter = 0;
                bandwidthFilter0.Frequency(BandwidthSmooth);
                bandwidthFilter1.Frequency(BandwidthSmooth);
                damping0.Frequency(DampingSmooth);
                damping1.Frequency(DampingSmooth);
            }
            ++ControlRateCounter;
            predelay.SetLength(PredelaySmooth);
            Density2 = DecaySmooth + 0.15;
            if (Density2 > 0.5)
                Density2 = 0.5;
            if (Density2 < 0.25)
                Density2 = 0.25;
            allpassFourTap1.SetFeedback(Density2);
            allpassFourTap3.SetFeedback(Density2);
            allpassFourTap0.SetFeedback(Density1);
            allpassFourTap2.SetFeedback(Density1);
            T bandwidthLeft = bandwidthFilter0(left) ;
            T bandwidthRight = bandwidthFilter1(right) ;
            T earlyReflectionsL = earlyReflectionsDelayLine0 ( bandwidthLeft * 0.5 + bandwidthRight * 0.3 )
                                + earlyReflectionsDelayLine0.GetIndex(2) * 0.6
                                + earlyReflectionsDelayLine0.GetIndex(3) * 0.4
                                + earlyReflectionsDelayLine0.GetIndex(4) * 0.3
                                + earlyReflectionsDelayLine0.GetIndex(5) * 0.3
                                + earlyReflectionsDelayLine0.GetIndex(6) * 0.1
                                + earlyReflectionsDelayLine0.GetIndex(7) * 0.1
                                + ( bandwidthLeft * 0.4 + bandwidthRight * 0.2 ) * 0.5 ;
            T earlyReflectionsR = earlyReflectionsDelayLine1 ( bandwidthLeft * 0.3 + bandwidthRight * 0.5 )
                                + earlyReflectionsDelayLine1.GetIndex(2) * 0.6
                                + earlyReflectionsDelayLine1.GetIndex(3) * 0.4
                                + earlyReflectionsDelayLine1.GetIndex(4) * 0.3
                                + earlyReflectionsDelayLine1.GetIndex(5) * 0.3
                                + earlyReflectionsDelayLine1.GetIndex(6) * 0.1
                                + earlyReflectionsDelayLine1.GetIndex(7) * 0.1
                                + ( bandwidthLeft * 0.2 + bandwidthRight * 0.4 ) * 0.5 ;
            T predelayMonoInput = predelay(( bandwidthRight + bandwidthLeft ) * 0.5f);
            T smearedInput = predelayMonoInput;
            //for(int j=0;j<4;j++)
            //    smearedInput = allpass[j] ( smearedInput );
	    smearedInput = allpass0(smearedInput);
	    smearedInput = allpass1(smearedInput);
	    smearedInput = allpass2(smearedInput);
	    smearedInput = allpass3(smearedInput);
            T leftTank = allpassFourTap0 ( smearedInput + PreviousRightTank ) ;
            leftTank = staticDelayLine0 (leftTank);
            leftTank = damping0(leftTank);
            leftTank = allpassFourTap1(leftTank);
            leftTank = staticDelayLine1(leftTank);
            T rightTank = allpassFourTap2 (smearedInput + PreviousLeftTank) ;
            rightTank = staticDelayLine2(rightTank);
            rightTank = damping1 (rightTank);
            rightTank = allpassFourTap3(rightTank);
            rightTank = staticDelayLine3(rightTank);
            PreviousLeftTank = leftTank * DecaySmooth;
            PreviousRightTank = rightTank * DecaySmooth;
            T accumulatorL = (0.6*staticDelayLine2.GetIndex(1))
                            +(0.6*staticDelayLine2.GetIndex(2))
                            -(0.6*allpassFourTap3.GetIndex(1))
                            +(0.6*staticDelayLine3.GetIndex(1))
                            -(0.6*staticDelayLine0.GetIndex(1))
                            -(0.6*allpassFourTap1.GetIndex(1))
                            -(0.6*staticDelayLine1.GetIndex(1));
            //T accumulatorR = (0.6*staticDelayLine0.GetIndex(2))
            //                +(0.6*staticDelayLine0.GetIndex(3))
            //                -(0.6*allpassFourTap1.GetIndex(2))
            //                +(0.6*staticDelayLine1.GetIndex(2))
            //                -(0.6*staticDelayLine2.GetIndex(3))
            //                -(0.6*allpassFourTap3.GetIndex(2))
            //                -(0.6*staticDelayLine3.GetIndex(2));
            accumulatorL = ((accumulatorL * EarlyMix) + ((1 - EarlyMix) * earlyReflectionsL));
            //accumulatorR = ((accumulatorR * EarlyMix) + ((1 - EarlyMix) * earlyReflectionsR));
            left = ( left + MixSmooth * ( accumulatorL - left ) ) * Gain;
            //right = ( right + MixSmooth * ( accumulatorR - right ) ) * Gain;
            outputs[0][i] = left;
            outputs[1][i] = left;
        }
    }

    void reset(){
        ControlRateCounter = 0;
        bandwidthFilter0.SetSampleRate (SampleRate );
        bandwidthFilter1.SetSampleRate (SampleRate );
        bandwidthFilter0.Reset();
        bandwidthFilter1.Reset();
        damping0.SetSampleRate (SampleRate );
        damping1.SetSampleRate (SampleRate );
        damping0.Reset();
        damping1.Reset();
        predelay.Clear();
        predelay.SetLength(PreDelayTime);
        allpass0.Clear();
        allpass1.Clear();
        allpass2.Clear();
        allpass3.Clear();
        allpass0.SetLength (0.0048 * SampleRate);
        allpass1.SetLength (0.0036 * SampleRate);
        allpass2.SetLength (0.0127 * SampleRate);
        allpass3.SetLength (0.0093 * SampleRate);
        allpass0.SetFeedback (0.75);
        allpass1.SetFeedback (0.75);
        allpass2.SetFeedback (0.625);
        allpass3.SetFeedback (0.625);
        allpassFourTap0.Clear();
        allpassFourTap1.Clear();
        allpassFourTap2.Clear();
        allpassFourTap3.Clear();
        allpassFourTap0.SetLength(0.020 * SampleRate * Size);
        allpassFourTap1.SetLength(0.060 * SampleRate * Size);
        allpassFourTap2.SetLength(0.030 * SampleRate * Size);
        allpassFourTap3.SetLength(0.089 * SampleRate * Size);
        allpassFourTap0.SetFeedback(Density1);
        allpassFourTap1.SetFeedback(Density2);
        allpassFourTap2.SetFeedback(Density1);
        allpassFourTap3.SetFeedback(Density2);
        allpassFourTap0.SetIndex(0,0,0,0);
        allpassFourTap1.SetIndex(0,0.006 * SampleRate * Size, 0.041 * SampleRate * Size, 0);
        allpassFourTap2.SetIndex(0,0,0,0);
        allpassFourTap3.SetIndex(0,0.031 * SampleRate * Size, 0.011 * SampleRate * Size, 0);
        staticDelayLine0.Clear();
        staticDelayLine1.Clear();
        staticDelayLine2.Clear();
        staticDelayLine3.Clear();
        staticDelayLine0.SetLength(0.15 * SampleRate * Size);
        staticDelayLine1.SetLength(0.12 * SampleRate * Size);
        staticDelayLine2.SetLength(0.14 * SampleRate * Size);
        staticDelayLine3.SetLength(0.11 * SampleRate * Size);
        staticDelayLine0.SetIndex(0, 0.067 * SampleRate * Size, 0.011 * SampleRate * Size , 0.121 * SampleRate * Size);
        staticDelayLine1.SetIndex(0, 0.036 * SampleRate * Size, 0.089 * SampleRate * Size , 0);
        staticDelayLine2.SetIndex(0, 0.0089 * SampleRate * Size, 0.099 * SampleRate * Size , 0);
        staticDelayLine3.SetIndex(0, 0.067 * SampleRate * Size, 0.0041 * SampleRate * Size , 0);
        earlyReflectionsDelayLine0.Clear();
        earlyReflectionsDelayLine1.Clear();
        earlyReflectionsDelayLine0.SetLength(0.089 * SampleRate);
        earlyReflectionsDelayLine0.SetIndex (0, 0.0199*SampleRate, 0.0219*SampleRate, 0.0354*SampleRate,0.0389*SampleRate, 0.0414*SampleRate, 0.0692*SampleRate, 0);
        earlyReflectionsDelayLine1.SetLength(0.069 * SampleRate);
        earlyReflectionsDelayLine1.SetIndex (0, 0.0099*SampleRate, 0.011*SampleRate, 0.0182*SampleRate,0.0189*SampleRate, 0.0213*SampleRate, 0.0431*SampleRate, 0);
    }

    void setParameter(int index, T value){
        switch(index){
            case DAMPINGFREQ:
                    DampingFreq =  1. - value;
                    break;
            case DENSITY:
                    Density1 = value;
                    break;
            case BANDWIDTHFREQ:
                    BandwidthFreq = value;
                    break;
            case PREDELAY:
                    PreDelayTime = value;
                    break;
            case SIZE:
                    Size = (0.95 * value) + 0.05;
					allpassFourTap0.Clear();
					allpassFourTap1.Clear();
					allpassFourTap2.Clear();
					allpassFourTap3.Clear();
                    allpassFourTap0.SetLength(0.020 * SampleRate * Size);
                    allpassFourTap1.SetLength(0.060 * SampleRate * Size);
                    allpassFourTap2.SetLength(0.030 * SampleRate * Size);
                    allpassFourTap3.SetLength(0.089 * SampleRate * Size);
                    allpassFourTap1.SetIndex(0,0.006 * SampleRate * Size, 0.041 * SampleRate * Size, 0);
                    allpassFourTap3.SetIndex(0,0.031 * SampleRate * Size, 0.011 * SampleRate * Size, 0);
					staticDelayLine0.Clear();
					staticDelayLine1.Clear();
					staticDelayLine2.Clear();
					staticDelayLine3.Clear();
                    staticDelayLine0.SetLength(0.15 * SampleRate * Size);
                    staticDelayLine1.SetLength(0.12 * SampleRate * Size);
                    staticDelayLine2.SetLength(0.14 * SampleRate * Size);
                    staticDelayLine3.SetLength(0.11 * SampleRate * Size);
                    staticDelayLine0.SetIndex(0, 0.067 * SampleRate * Size, 0.011 * SampleRate * Size , 0.121 * SampleRate * Size);
                    staticDelayLine1.SetIndex(0, 0.036 * SampleRate * Size, 0.089 * SampleRate * Size , 0);
                    staticDelayLine2.SetIndex(0, 0.0089 * SampleRate * Size, 0.099 * SampleRate * Size , 0);
                    staticDelayLine3.SetIndex(0, 0.067 * SampleRate * Size, 0.0041 * SampleRate * Size , 0);
                    break;
            case DECAY:
                    Decay = value;
                    break;
            case GAIN:
                    Gain = value;
                    break;
            case MIX:
                    Mix = value;
                    break;
            case EARLYMIX:
                    EarlyMix = value;
                    break;
        }
    }

    float getParameter(int index){
        switch(index){
            case DAMPINGFREQ:
                    return DampingFreq * 100.;
                    break;
            case DENSITY:
                    return Density1 * 100.f;
                    break;
            case BANDWIDTHFREQ:
                    return BandwidthFreq * 100.;
                    break;
            case PREDELAY:
                    return PreDelayTime * 100.;
                    break;
            case SIZE:
                    return (((0.95 * Size) + 0.05)*100.);
                    break;
            case DECAY:
                    return Decay * 100.f;
                    break;
            case GAIN:
                    return Gain * 100.f;
                    break;
            case MIX:
                    return Mix * 100.f;
                    break;
            case EARLYMIX:
                    return EarlyMix * 100.f;
                    break;
            default: return 0.f;
                break;

        }
    }

    void setSampleRate(T sr){
        SampleRate = sr;
        ControlRate = SampleRate / 1000;
        reset();
    }
};



template<typename T, int maxLength>
class Allpass
{
private:
    T buffer[maxLength];
	int index;
	int Length;
	T Feedback;

public:
    Allpass()
    {
		SetLength ( maxLength - 1 );
		Clear();
		Feedback = 0.5;
    }

	T operator()(T input)
    {
		T output;
		T bufout;
		bufout = buffer[index];
		T temp = input * -Feedback;
		output = bufout + temp;
		buffer[index] = input + ((bufout+temp)*Feedback);
		if(++index>=Length) index = 0;
		return output;

    }

	void SetLength (int Length)
    {
       if( Length >= maxLength )
			Length = maxLength;
	   if( Length < 0 )
			Length = 0;

        this->Length = Length;
    }

	void SetFeedback(T feedback)
    {
        Feedback = feedback;
    }

    void Clear()
    {
        memset(buffer, 0, sizeof(buffer));
		index = 0;
    }

    int GetLength() const
    {
        return Length;
    }
};

template<typename T, int maxLength>
class StaticAllpassFourTap
{
private:
    T buffer[maxLength];
	int index1, index2, index3, index4;
	int Length;
	T Feedback;

public:
    StaticAllpassFourTap()
    {
		SetLength ( maxLength - 1 );
		Clear();
		Feedback = 0.5;
    }

	T operator()(T input)
    {
		T output;
		T bufout;

		bufout = buffer[index1];
		T temp = input * -Feedback;
		output = bufout + temp;
		buffer[index1] = input + ((bufout+temp)*Feedback);

		if(++index1>=Length)
			index1 = 0;
		if(++index2 >= Length)
			index2 = 0;
		if(++index3 >= Length)
			index3 = 0;
		if(++index4 >= Length)
			index4 = 0;

		return output;

    }

	void SetIndex (int Index1, int Index2, int Index3, int Index4)
	{
		index1 = Index1;
		index2 = Index2;
		index3 = Index3;
		index4 = Index4;
	}

	T GetIndex (int Index)
	{
		switch (Index)
		{
			case 0:
				return buffer[index1];
				break;
			case 1:
				return buffer[index2];
				break;
			case 2:
				return buffer[index3];
				break;
			case 3:
				return buffer[index4];
				break;
			default:
				return buffer[index1];
				break;
		}
	}

	void SetLength (int Length)
    {
       if( Length >= maxLength )
			Length = maxLength;
	   if( Length < 0 )
			Length = 0;

        this->Length = Length;
    }


    void Clear()
    {
        memset(buffer, 0, sizeof(buffer));
		index1 = index2  = index3 = index4 = 0;
    }

	void SetFeedback(T feedback)
    {
        Feedback = feedback;
    }


    int GetLength() const
    {
        return Length;
    }
};

template<typename T, int maxLength>
class StaticDelayLine
{
private:
    T buffer[maxLength];
	int index;
	int Length;
	T Feedback;

public:
    StaticDelayLine()
    {
		SetLength ( maxLength - 1 );
		Clear();
    }

	T operator()(T input)
    {
		T output = buffer[index];
		buffer[index++] = input;
		if(index >= Length)
			index = 0;
		return output;

    }

	void SetLength (int Length)
    {
       if( Length >= maxLength )
			Length = maxLength;
	   if( Length < 0 )
			Length = 0;

        this->Length = Length;
    }

    void Clear()
    {
        memset(buffer, 0, sizeof(buffer));
		index = 0;
    }

    int GetLength() const
    {
        return Length;
    }
};

template<typename T, int maxLength>
class StaticDelayLineFourTap
{
private:
    T buffer[maxLength];
	int index1, index2, index3, index4;
	int Length;
	T Feedback;

public:
    StaticDelayLineFourTap()
    {
		SetLength ( maxLength - 1 );
		Clear();
    }

	//get ouput and iterate
	T operator()(T input)
    {
		T output = buffer[index1];
		buffer[index1++] = input;
		if(index1 >= Length)
			index1 = 0;
		if(++index2 >= Length)
			index2 = 0;
		if(++index3 >= Length)
			index3 = 0;
		if(++index4 >= Length)
			index4 = 0;
		return output;

    }

	void SetIndex (int Index1, int Index2, int Index3, int Index4)
	{
		index1 = Index1;
		index2 = Index2;
		index3 = Index3;
		index4 = Index4;
	}


	T GetIndex (int Index)
	{
		switch (Index)
		{
			case 0:
				return buffer[index1];
				break;
			case 1:
				return buffer[index2];
				break;
			case 2:
				return buffer[index3];
				break;
			case 3:
				return buffer[index4];
				break;
			default:
				return buffer[index1];
				break;
		}
	}


	void SetLength (int Length)
    {
       if( Length >= maxLength )
			Length = maxLength;
	   if( Length < 0 )
			Length = 0;

        this->Length = Length;
    }


    void Clear()
    {
        memset(buffer, 0, sizeof(buffer));
		index1 = index2  = index3 = index4 = 0;
    }


    int GetLength() const
    {
        return Length;
    }
};

template<typename T, int maxLength>
class StaticDelayLineEightTap
{
private:
    T buffer[maxLength];
	int index1, index2, index3, index4, index5, index6, index7, index8;
	int Length;
	T Feedback;

public:
    StaticDelayLineEightTap()
    {
		SetLength ( maxLength - 1 );
		Clear();
    }

	//get ouput and iterate
	T operator()(T input)
    {
		T output = buffer[index1];
		buffer[index1++] = input;
		if(index1 >= Length)
			index1 = 0;
		if(++index2 >= Length)
			index2 = 0;
		if(++index3 >= Length)
			index3 = 0;
		if(++index4 >= Length)
			index4 = 0;
		if(++index5 >= Length)
			index5 = 0;
		if(++index6 >= Length)
			index6 = 0;
		if(++index7 >= Length)
			index7 = 0;
		if(++index8 >= Length)
			index8 = 0;
		return output;

    }

	void SetIndex (int Index1, int Index2, int Index3, int Index4, int Index5, int Index6, int Index7, int Index8)
	{
		index1 = Index1;
		index2 = Index2;
		index3 = Index3;
		index4 = Index4;
		index5 = Index5;
		index6 = Index6;
		index7 = Index7;
		index8 = Index8;
	}


	T GetIndex (int Index)
	{
		switch (Index)
		{
			case 0:
				return buffer[index1];
				break;
			case 1:
				return buffer[index2];
				break;
			case 2:
				return buffer[index3];
				break;
			case 3:
				return buffer[index4];
				break;
            case 4:
				return buffer[index5];
				break;
			case 5:
				return buffer[index6];
				break;
			case 6:
				return buffer[index7];
				break;
			case 7:
				return buffer[index8];
				break;
			default:
				return buffer[index1];
				break;
		}
	}


	void SetLength (int Length)
    {
       if( Length >= maxLength )
			Length = maxLength;
	   if( Length < 0 )
			Length = 0;

        this->Length = Length;
    }


    void Clear()
    {
        memset(buffer, 0, sizeof(buffer));
		index1 = index2  = index3 = index4 = index5 = index6 = index7 = index8 = 0;
    }


    int GetLength() const
    {
        return Length;
    }
};

template<typename T, int OverSampleCount>
    class StateVariable
    {
    public:

        enum FilterType
        {
            LOWPASS,
            HIGHPASS,
            BANDPASS,
            NOTCH,
            FilterTypeCount
        };

    private:

        T sampleRate;
        T frequency;
        T q;
        T f;

        T low;
        T high;
        T band;
        T notch;

        T *out;

    public:
        StateVariable()
        {
            SetSampleRate(44100.);
            Frequency(1000.);
            Resonance(0);
            Type(LOWPASS);
            Reset();
        }

        T operator()(T input)
        {
            for(unsigned int i = 0; i < OverSampleCount; i++)
            {
                low += f * band + 1e-25;
                high = input - low - q * band;
                band += f * high;
                notch = low + high;
            }
			return *out;
        }

        void Reset()
        {
            low = high = band = notch = 0;
        }

        void SetSampleRate(T sampleRate)
        {
            this->sampleRate = sampleRate * OverSampleCount;
            UpdateCoefficient();
        }

        void Frequency(T frequency)
        {
            this->frequency = frequency;
            UpdateCoefficient();
        }

        void Resonance(T resonance)
        {
            this->q = 2 - 2 * resonance;
        }

        void Type(int type)
        {
            switch(type)
            {
            case LOWPASS:
                out = &low;
                break;

            case HIGHPASS:
                out = &high;
                break;

            case BANDPASS:
                out = &band;
                break;

            case NOTCH:
                out = &notch;
                break;

            default:
                out = &low;
                break;
            }
        }

    private:
        void UpdateCoefficient()
        {
            f = 2. * sinf(3.141592654 * frequency / sampleRate);
        }
	};

#endif
