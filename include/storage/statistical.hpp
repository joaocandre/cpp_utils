#ifndef NUMERICAL_HPP
#define NUMERICAL_HPP

#include <vector>
#include <matrix>
#include <volume>
#include <numeric>
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */

namespace std {
namespace statistical {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// general purpose / utils
inline double norm(double x,double y){ return sqrt(pow(x,2)+pow(y,2)); }
inline double norm(double x,double y, double z){ return sqrt(pow(x,2)+pow(y,2)+pow(z,2)); }
inline double deg2rad(double deg){ return deg * M_PI / 180.0; }
inline double rad2deg(double rad){ return rad * 180.0 / M_PI; }
inline double sign(double value) {
	if(value < 0) return -1.0;
	else return 1.0;
}
inline double clamp(double value, double min, double max) {
  if (min > max) {
    assert(0);
    return value;
  }
  return value < min ? min : value > max ? max : value;
}
inline double round(double val, int dig) {	
	double vv, mult, rnd;
	mult=1.0;
	for (int i = 0; i < dig; ++i) mult=mult*10;
	vv=mult*val;
	
    if( vv < 0 ) rnd=ceil(vv - 0.5);
    else rnd=floor(vv + 0.5);

    return (rnd/mult);
}
/////////////////////////////////////////////////////////////////////////////
int findPeaks(std::vector<float>& _output, std::vector<float>& _input, int _height, int _width){
	int n=_input.size();
	if (n<=0 || _height<1 || _width<1) return 1;

	bool peak=false;
	int c=0;
	int imax=0;
	float max=_height;

	_output.resize(0);
	//VectorXf inds;
	std::vector<float> _maxs(0,0.0);
	// VectorXf _maxs=VectorXf::Zero(0);

	for (int i = 0; i < n; ++i){
		if (_input[i]>=_height) {
			// POTENTIAL PEAK
			c++;
			if (_input[i]>=max){
				// at the start of any peak it!!
				//printf("new max! i %d \n",i);
				max=_input[i];
				imax=i;
				if (peak) {
					// place values on container (only if inside a peak and a new maximum is found)
					_output[_output.size()-1]=imax+0.0;
					_maxs[_output.size()-1]=max;
					//printf("peak value updated i %d\n",i);
				}
			}
			if (c>=_width){
				// IS A PEAK
				if (!peak) {
					printf("new peak! i %d! last imax %d\n",i,imax);
					peak=true;
					// increase container size
					//printf("enlarging array %d to %d\n",_output.rows(),_output.rows()+1);
					_output.resize(_output.size()+1);
					_maxs.resize(_output.size()+1);
					//printf("placing value %d at pos %d\n",imax,_output.rows()-1);
					_output[_output.size()-1]=imax;
					_maxs[_output.size()-1]=max;
				}				
			} 
		} else {
			// NOT A PEAK
			//printf("no peak! i%d\n",i);
			c=0;
			peak=false;
			max=_height;
		}

	// at the end of the loop maxs and inds are populated by the maximum vaues in each peak and index
	}

	// avg_cycle=0.0;
	// for (int i = 1; i < _output.rows(); ++i) avg_cycle+=(_output(i)-_output(i-1));
	// avg_cycle=avg_cycle/_output.rows();

	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float vectorDistance (std::vector<float> _ref, std::vector<float> _sol) {
	// if (_ref.size() < _sol.size()) _ref.resize(_sol.size());
	// else if (_sol.size() < _ref.size()) _sol.resize(_ref.size());
	if (_ref.size() != _sol.size()) return std::numeric_limits<float>::max();

	float distance=0.0;
	for (int instant = 0; instant < _ref.size(); ++instant)	{
		distance+=sqrt(pow(_ref[instant]-_sol[instant],2)); 
	}
	return distance;
};
/////////////////////////////////////////////////////////////////////////////
float vectorAverage(float& _avg, std::vector<float>& _input){
	float sum=0.0;
	float size=(float)(_input.size());
	for (int i=0; i< _input.size(); i++) sum+=_input[i];
	return (sum/size);
}
/////////////////////////////////////////////////////////////////////////////
template <typename _type>
_type average(std::vector<_type>& _input){
	_type avg;
	_type ss = (_type)_input.size();
	for (auto& val : _input) avg += val;
	return (avg/ss);
}
template <typename _type>
_type average(std::matrix<_type>& _input){
	return average(_input.vector());
}
template <typename _type>
_type average(std::volume<_type>& _input){
	return average(_input.vector());
}
/////////////////////////////////////////////////////////////////////////////
float vectorStdDeviation(float& _std, std::vector<float>& _input){
	float avg = 0.0;
	vectorAverage(avg,_input);
	float w=(float)(1.0/_input.size());
	for (auto& val : _input) avg += w*val;
	return avg;
}
/////////////////////////////////////////////////////////////////////////////
int reSample(std::vector<float>& _input, std::vector<float>& _output, int _size){
	// printf("resampling %d to %d\n", _input.size(), _size);
	int _prev_size=_input.size();
	if (_size>1 && _prev_size>0 && _prev_size!=_size) {
		
		std::vector<float> ids (_size,0.0);
		float step=(_prev_size-1.0)/_size;
		for (int i = 0; i < ids.size(); ++i) ids[i]=i*step;

		//_output.resize(_size);//,0.0);
		_output.assign(_size,0.0);

		_output[0]=_input[0];
		_output[_size-1]=_input[_prev_size-1];

		int li=0;
		int ui=0;
		float sl=0.0;

		for (int i = 0; i < _size; ++i){
			li=(int)(ids[i]);
			ui=(int)(ids[i]+1.0);

			if (ui==_prev_size) {
				ui--;
				li--;
			}			
			sl=(_input[ui]-_input[li])/(ui-li);
			_output[i]=_input[li]+sl*(ids[i]-((int)(ids[i])));
			if (i==0) printf("li %d(%f) ui %d(%f)\n",li,_input[li],ui,_input[ui]);

		}
	} else return 1;
	return 0;
}	
/////////////////////////////////////////////////////////////////////////////
int reSample(VectorXf& _input, VectorXf& _output, int _size){
	//printf("resampling %d to %d\n", _input.rows(), _size);
	int _prev_size=_input.rows();
	if (_size>1 && _prev_size>0 && _prev_size!=_size) {
		
		VectorXf ids=Eigen::VectorXf::LinSpaced(_size,0,_prev_size-1);

		_output=VectorXf::Zero(_size);

		_output(0)=_input(0);
		_output(_size-1)=_input(_prev_size-1);

		int li=0;
		int ui=0;
		float sl=0.0;

		//std::cout << ids << std::endl;
		//printf("sadsdadsadsada----çsad\n");
		for (int i = 0; i < _size; ++i){
			li=(int)(ids(i));
			ui=(int)(ids(i)+1.0);

			if (ui==_prev_size) {
				ui--;
				li--;
			}

			///printf("%d -- %d\n",li,ui);

			sl=(_input(ui)-_input(li))/(ui-li);
			_output(i)=_input(li)+sl*(ids(i)-((int)(ids(i))));
		}
	} else return 1;

	return 0;
}
/////////////////////////////////////////////////////////////////////////////
int reSample2(std::vector<float>& _inputRef, std::vector<float>& _inputVal, std::vector<float>& _outputRef, std::vector<float>& _outputVal, bool _periodic){
	// printf("resampling %d to %d\n", _input.size(), _size);
	// assumes _inputRef and _outputRef are sorted!!!!
	//bool _periodic=true;
	int prev_size=_inputVal.size();
	int new_size=_outputRef.size();
	if (new_size>1 && prev_size>0 && _inputRef.size()==prev_size) {
		
		// ratios between vector size (1 id in new vector = X in old vector)
		std::vector<float> ids (new_size,0.0);
		float step=(prev_size-1.0)/new_size;
		for (int i = 0; i < ids.size(); ++i) ids[i]=i*step;

		//_output.resize(_size);//,0.0);
		_outputVal.assign(new_size,0.0);

		int li=0;
		int ui=0;
		float sl=0.0;
		for (int i = 0; i < new_size; ++i){
			// find li and ui
			while (_outputRef[i]>_inputRef[ui] && ui<_inputRef.size()-1) ui++;
			li=ui;
			while (_outputRef[i]<_inputRef[li] && li>0) li--;
			// range check
			if (li==ui && _outputRef[i] == _inputRef[li]) {
				// 1) _outputRef[i] == _inputRef[li]
				_outputVal[i]=_inputVal[ui];
				// printf("%d, %f: li %d(%f) ui %d(%f); %f --> %f\n",i,_outputRef[i],li,_inputRef[li],ui,_inputRef[ui],_inputVal[li],_outputVal[i]);
				continue;
			} else if (li==ui && _periodic) {
				// 2) _outputRef[i] out of _inputRef bounds
				if (_outputRef[i] < _inputRef[li]) li=prev_size-1;
				if (_outputRef[i] > _inputRef[li]) ui=0;
				sl=(_inputVal[ui]-_inputVal[li])/(abs(_inputRef[ui])-abs(_inputRef[li]));
				_outputVal[i]=_inputVal[li]+sl*(abs(_outputRef[i])-abs(_inputRef[li]));	
				//printf("*%d, %f: li %d(%f), ui %d(%f), slope %f; %f --> %f\n",i,_outputRef[i],li,_inputRef[li],ui,_inputRef[ui],sl,_inputVal[li],_outputVal[i]);		
			} else {
				// compute slope and val
				sl=(_inputVal[ui]-_inputVal[li])/(_inputRef[ui]-_inputRef[li]);
				_outputVal[i]=_inputVal[li]+sl*(_outputRef[i]-_inputRef[li]);		
				//printf("%d, %f: li %d(%f), ui %d(%f), slope %f; %f --> %f\n",i,_outputRef[i],li,_inputRef[li],ui,_inputRef[ui],sl,_inputVal[li],_outputVal[i]);		
			}
		}
	} else return 1;
	return 0;
}	
/////////////////////////////////////////////////////////////////////////////
VectorXf kfolds(int _samples, int _nfolds){
	VectorXf labels=VectorXf::Zero(_samples);
	for (int i = 0; i < _samples; ++i) labels(i)=rand() % _nfolds;
	return labels;
}	
/////////////////////////////////////////////////////////////////////////////
int rocTest(int _metric, VectorXf _input){
	/// ...
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
//int pca();

/////////////////////////////////////////////////////////////////////////////
float weighted2DMean(const MatrixXf& _input, const VectorXf& _weights){
	float mn=0.0;
	for (int i = 0; i < _input.rows(); ++i){
		mn+=_weights(i)*(_input.row(i).mean());
	}
	return mn;
}
////////////////////////////////////////////////////////////////////////////
int meanFilter(const std::vector<float>& _ref, const std::vector<float>& _input, std::vector<float>& _output, int _filter_type){
//low pass average field
	//weighted average with neightbours
	_output.assign (_input.size(),0.0);
	if (_filter_type==1) {
		//printf("f1\n");
		//method 1 (small window)
		float lw,hw;
		//first & last vals (wrap around, assumes periodicity)
		lw=1.0-((_ref[0]-_ref[_ref.size()-1])/(_ref[1]-_ref[_ref.size()-1]));
		hw=1.0-((_ref[1]-_ref[0])/(_ref[1]-_ref[_ref.size()-1]));
		_output[0]=lw*_input[_input.size()-1]+hw*_input[1];
		//printf("f0 %f (lw: %f _input-1: %f hw: %f _input+1: %f)\n",_output[0],lw,_input[_input.size()-1],hw,_input[1]);
		lw=1.0-((_ref[_ref.size()-1]-_ref[_ref.size()-2])/(_ref[0]-_ref[_ref.size()-2]));
		hw=1.0-((_ref[0]-_ref[_ref.size()-1])/(_ref[0]-_ref[_ref.size()-2]));
		_output[_output.size()-1]=lw*_input[_input.size()-2]+hw*_input[0];
		for (int i = 1; i < _ref.size()-1; ++i){
			lw=1.0-((_ref[i]-_ref[i-1])/(_ref[i+1]-_ref[i-1]));
			hw=1.0-((_ref[i+1]-_ref[i])/(_ref[i+1]-_ref[i-1]));
			_output[i]=lw*_input[i-1]+hw*_input[i+1];
			//printf("_input: %f, _input[-1]: %f, _input[+1]: %f, lw: %f, hw: %f, _output: %f\n",_ref[i],_ref[i-1],_ref[i+1],lw,hw,_output[i]);
		}
		//printf("fE %f\n",_output[_output.size()-1]);

	} else if (_filter_type==2) {
		//printf("f2\n");
		//method 2 (large variable window)	
		int wsize=20;
		float dtotal=0.0;
		//std::vector<float> f2y (_ref.size(),0.0);
		std::vector<float> lws (wsize,0.0);
		std::vector<float> hws (wsize,0.0);
		for (int i = 0; i < wsize; ++i){
			_output[i]=_input[i];
			_output[_output.size()-1-i]=_input[_input.size()-1-i];
		}
		for (int i = wsize; i < _ref.size()-wsize; ++i){
			// compute distances
			for (int j = 0; j < lws.size(); ++j){ 
				lws[j]=1.0/abs(_ref[i-wsize+j]-_ref[i]); 
				hws[j]=1.0/abs(_ref[i+wsize-j]-_ref[i]);
				dtotal+=(lws[j]+hws[j]);
			}
			// compute value
			for (int j = 0; j < lws.size(); ++j){ 
				//printf("pass %d, vals %f and %f, weights %f and %f -> \n",j,_input[i-wsize+j],_input[i+wsize-j],(lws[j]/dtotal),(hws[j]/dtotal) );
				_output[i]+=(lws[j]/dtotal)*_input[i-wsize+j]+(hws[j]/dtotal)*_input[i+wsize-j];
			}		
			//printf("ref %f _input %f f2y %f\n",_ref[i],_input[i],f2y[i]);
			dtotal=0.0;
		}
		//plt::named_plot("filtered #2 values",_ref,f2y);
	}
	//plt::named_plot("filtered values", _ref,_output);
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
// template functions that sorts a vec tor by indexes
// from https://stackoverflow.com/questions/1577475/c-sorting-and-keeping-track-of-indexes
template <typename T>
std::vector<size_t> sort_indexes(std::vector<T> &v) {
  // initialize original index locations
  std::vector<size_t> idx(v.size());
  std::iota(idx.begin(), idx.end(), 0);
  // sort indexes based on comparing values in v
  std::sort(idx.begin(), idx.end(),  [&v](size_t i1, size_t i2) {return v[i1] < v[i2];});
  return idx;
}
////////////////////////////////////////////////////////////////////////////
std::vector<float> periodicInterpolation(std::vector<float> _x, std::vector<float> _y, std::vector<float> _ref, bool _filtering){
	//ideally, supply vector with desired phase values/indexes, output resampled/interpolated values
	//input values may be too close, info may overlap, as such a precision value is set, values are rounded and then averaged
	//this also helps with periodic signals with a bad/noisy period or local outlier data points
	//a more accurate way would be to just average the slopes of neightbour pairs; function is inneficient and needs rewrite!
	//results are very dependent on the quality of input data (periodicy)
	//sx: sorted _x values
	//sy: sorted _y values
	//fy: filtered and sorted _y values
	//ry: resampled, filtered and sorted _y values


	//sort vectors (by phase)
	std::vector<float> sx (_x.size(),0.0);
	std::vector<float> sy (_y.size(),0.0);
	std::vector<long unsigned int> _ids = sort_indexes(_x);
	for (int i = 0; i < _ids.size(); ++i){
		sx[i]=_x[_ids[i]];
		sy[i]=_y[_ids[i]];
	}

	// smoth output data (optional) to ensure all samples contribute to final solution
	// method 2 should be used if there is chance of value repetition
	std::vector<float> fy(sy.size(),0.0);
	if (_filtering) meanFilter(sx,sy,fy,2);
	else fy=sy;

	// resample dataset
	std::vector<float> _output;
	std::vector<float> ry (sx.size());
	reSample2(sx,fy,_ref,_output,true);

	//plt::named_plot("ggg",_ref,_output);
	//plt::legend();
	//plt::show();
    return _output;
}

////////////////////////////////////////////////////////////////////////////
} 
}
#endif //NUMERICAL_HPP