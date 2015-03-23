#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <math.h>
#include <algorithm>
//#include"persistence1d.hpp"
#include "digital_filter.h"
#include "wavmonopcm.h"
#include "FFTReal.h"



using namespace upc;
using namespace std;
//using namespace p1d;



const float A=300;
const float window=160; //rate*10e-3
const float frame_dur = 0.3;
#define nf0 3
const float f0[nf0] = {85,85,0}; 
const float pitch_factor = 1;
const float primer[5]={850,750,400,650,375};
const float segon[5]={1300,1725,2225,1100,750};
const string names[5]={"a","e","i","o","u"};


int main(int argc, char **argv) {
	DigitalFilter F1, F2;
	float rate=16e3;
	float To;//=160;

	vector<float> deltes;
	vector<float> llegit;
	vector<vector<float>::iterator> aux;
	vector<float> R;
	vector<float> x;
	vector<float> y;
	//	vector<float> f((1e-2)*rate);
	vector<float> buffer((1e-2)*rate);
//	Persistence1D p;


	//llegim el fitxer
	wavmonopcm_read("veu.wav",llegit,rate);
	cout<<"llegit te tamany: "<<llegit.size()<<endl<<"generarem una vocal sintetica del mateix tamany"<<endl;


	//llegim un tram
	for(std::vector<float>::iterator it=llegit.begin();it<llegit.end();it+=window){

		//copiem un tram del vector a un buffer
		copy(it,it+window,buffer.data());

		//calculem la autocorrelacio

		//metode 1
		//		operador->do_fft(&f[0],&buffer[0]);
		//		for(vector<float>::iterator acc=f.begin(); acc!=f.begin()+distance(f.begin(),f.end())/2 ; ++acc){
		//			*acc=sqrt(((*acc)*(*acc))+((*(acc+(distance(f.begin(),f.end())/2)))*(*(acc+(distance(f.begin(),f.end())/2))))); //la llibreria fftreal
		//		}
		//		while(f.size()<(1e-2)*rate){f.push_back(0.0);}
		//		operador->do_ifft(&f[0],&R[0]);
		//		f.clear();
		//		buffer.clear();
		//metode 2
		float acc;

		for(int i=0;i<(buffer.size()-1);i++){
			acc=0;
			for(vector<float>::iterator accj=buffer.begin();accj<buffer.end()-i;++accj){
				acc+=(*(accj+i))*(*accj);
			}
			cout<<acc<<endl;
			R.push_back(acc);
		}
		std::fill(buffer.begin(),buffer.end(),0);

		//busquem el pitch en el vector de correlacio R
		//amb wavesurfer hem vist com la frequencia aproximada del pitch es de 130 Hz per al nostre senyal de prova
		//metode 1
								vector<float>::iterator max_pos=max_element(R.begin(),R.end());
								vector<float>::iterator max_pos_2=max_element(max_pos+(rate/50),max_pos+(rate/50)+(rate/160));
								To=abs(distance(max_pos,max_pos_2));
		//metode 2
		//
		//				for(vector<float>::iterator i=max_pos;i<max_pos+rate/50+rate/400;++i){
		//						if(*(i+1)>*i && *(i+1)>*(i+2)){aux.push_back(i);}
		//
		//				}
		//
		//				for(vector<vector<float>::iterator>::iterator i=aux.begin();i<aux.end();++i){
		//					if(**i>0.5*(*max_pos)){To=distance(max_pos,*i);}
		//				}
		//
		//
		//				do{
		//					max_pos_2=max_element(max_pos_2+(rate/400),x.end());
		//				}while((*max_pos_2>*(max_pos_2+1)) && (*max_pos_2)>*(max_pos_2 - 1 ) && (*max_pos_2 < *max_pos*0.5));
		//metode 3 (usem la llibreria persistence1d)
//		Persistence1D p;
//
//		p.RunPersistence(R);
//		vector< TPairedExtrema > Extrema;
//		p.GetPairedExtrema(Extrema, 0.5);
//
//		To=Extrema.at(1).MaxIndex-Extrema.at(2).MaxIndex;
		//




//		cout<<To<<endl; //debug
		R.clear();

		//generem la excitacio amb el vector trobat
		while(deltes.size()<rate*1e-2){
			deltes.push_back(300.0);
			for(int i=0;i< To - 1;i++){deltes.push_back(0.0);}
		}

		/* Create resonator F1 and F2: set_resonator, set_gain */
		F1.set_resonator((primer[1]/rate),150/rate);
		F2.set_resonator(segon[1]/rate,150/rate);


		x=F2(F1(deltes));
		y.insert(y.end(),x.begin(),x.end());
		/* Save wav file */
		deltes.clear();
		x.clear();
	}
	cout<<"vocal generada amb exit"<<endl;

	return wavmonopcm_write("ordinador.wav", y, rate);

}

