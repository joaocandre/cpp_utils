#ifndef MPLEARN_INCLUDE_ETC_UTILS_HPP_
#define MPLEARN_INCLUDE_ETC_UTILS_HPP_

#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <cassert>

namespace math {

//------------------------------------------------------------------------------
/// @brief      Computes the norm of the *data* array.
///
/// @param[in]  data  Data array (i.e. pointer to first element)
/// @param[in]  N     Number of elements
///
/// @return     Norm of the data array (||data||).
///
inline double norm(const double * data, const int N) {
    double n = 0.0;
    for (int i = 0; i < N; ++i) {
        n += pow(data[i], 2);
    }
    return sqrt(n);
}


//------------------------------------------------------------------------------
/// @brief      RMSD (Root-mean-square deviation) value (i.e. Euclidean distance) between input vectors *data* and *reference*.
///
/// @param[in]  data       First data vector.
/// @param[in]  reference  Second data vector.
///
/// @return     Value of the RMSD metric between input vectors.
///
inline double rmsd(const std::vector< double >& data, const std::vector< double >& reference) {
    if (data.size() != reference.size()) {
        throw std::invalid_argument(std::string(__func__) + ": data and reference containers must have same size.");
    }
    double rmsd = 0.0;
    for (size_t idx = 0; idx < data.size(); idx++) {
        rmsd += sqrt(pow(data[idx] - reference[idx], 2));
        // rmsd += pow(data[idx] - reference[idx], 2);
    }
    return rmsd;
}


//------------------------------------------------------------------------------

inline std::vector< double > normalize(const std::vector< double >& data, const std::vector< double >& bounds = { }) {
    if (!data.size()) {
        throw std::invalid_argument(std::string(__func__) + ": invalid input data.");
    }
    double max;
    double min;
    if (bounds.empty()) {
        min = data[0];
        max = data[0];
        for (int idx = 1; idx < data.size(); idx++) {
            if (data[idx] > max) {
                max = data[idx];
            }
            else if (data[idx] < min) {
                min = data[idx];
            }
        }
    } else {
        min = bounds[0];
        max = bounds[1];
    }

    double range = max - min;
    std::vector< double > normalized(data.size());
    for (int idx = 1; idx < data.size(); idx++) {
        normalized[idx] =  (data[idx] - min) / (range);
    }

    return normalized;
}


//------------------------------------------------------------------------------

inline double rmsd2(const std::vector< double >& data, const std::vector< double >& reference) {
    if (data.size() != reference.size()) {
        throw std::invalid_argument(std::string(__func__) + ": data and reference containers must have same size.");
    }
    double max = data[0];
    double min = data[1];
    for (int idx = 1; idx < data.size(); idx++) {
        if (data[idx] > max) {
            max = data[idx];
        }
        else if (data[idx] < min) {
            min = data[idx];
        }
    }

    return rmsd(normalize(data, { min, max }), normalize(reference, { min, max }));
}


//------------------------------------------------------------------------------
/// @brief      Apply a mean (smoothing) filter to given *input*.
///
/// @param[in]  ref          Input x-values (required for window computation in case step/increment is not constant)
/// @param[in]  input        Input y-values (to be filtered)
/// @param[in]  filter_type  Either '1' -> (small fixed window) or '2' -> large variable window
/// @param[out] out          Container for output filtered/smoothed values.
///
/// @tparam     T            Output data type. Filtered values are cast to T.
///
/// @todo       Review implementation, improve documentation.
///
template < typename T >
void mean_filter(const std::vector< T >& ref, const std::vector< T >& input, size_t filter_type, std::vector< T >& out) {
    assert(ref.size() > 0);
    assert(ref.size() == input.size());
    assert(filter_type == 1 || filter_type == 2);
    // clear output vector
    out.assign(input.size(), 0.0);
    // low pass average field -> weighted neighbour average
    if (filter_type == 1) {  // method 1 (small window)
        T lw, hw;
        // first & last vals (wrap around, assumes periodicity)
        lw = 1.0 - ((ref[0] - ref.back()) / (ref[1] - ref.back()));
        hw = 1.0 - ((ref[1] - ref[0]) / (ref[1] - ref.back()));
        out[0] = lw * input.back() + hw * input[1];
        // printf("f0 %f (lw: %f input-1: %f hw: %f input+1: %f)\n",out[0],lw,input[input.size()-1],hw,input[1]);
        lw = 1.0-((ref.back() - ref[ref.size() - 2]) / (ref[0] - ref[ref.size() - 2]));
        hw = 1.0-((ref[0] - ref.back()) / (ref[0] - ref[ref.size() - 2]));
        out.back() = lw * input[input.size() - 2] + hw * input[0];
        for (int i = 1; i < ref.size() - 1; ++i) {
            lw = 1.0 - ((ref[i] - ref[i-1]) / (ref[i+1] - ref[i-1]));
            hw = 1.0 - ((ref[i+1] - ref[i]) / (ref[i+1] - ref[i-1]));
            out[i] = lw * input[i-1] + hw * input[i+1];
        }
    } else if (filter_type == 2) {  // method 2 (large variable window)
        size_t wsize = 20;
        T dtotal = 0.0;
        // std::vector<float> f2y (ref.size(),0.0);
        std::vector< T > lws(wsize, 0.0);
        std::vector< T > hws(wsize, 0.0);
        for (int i = 0; i < wsize; ++i) {
            out[i] = input[i];
            out[out.size()-1-i] = input[input.size()-1-i];
        }
        for (int i = wsize; i < ref.size()-wsize; ++i) {
            // compute distances
            for (int j = 0; j < lws.size(); ++j) {
                lws[j] = 1.0 / abs(ref[i-wsize+j]-ref[i]);
                hws[j] = 1.0 / abs(ref[i+wsize-j]-ref[i]);
                dtotal += (lws[j] + hws[j]);
            }
            // compute value
            for (int j = 0; j < lws.size(); ++j) {
                out[i] += (lws[j] / dtotal) * input[i-wsize+j] + (hws[j]/dtotal) * input[i+wsize-j];
            }
            dtotal = 0.0;
        }
    }
}

//------------------------------------------------------------------------------
/// @brief      Apply a mean (smoothing) filter to given *input* (inline initialization overload).
///
/// @param[in]  ref          Input x-values (required for window computation in case step/increment is not constant)
/// @param[in]  input        Input y-values (to be filtered)
/// @param[in]  filter_type  Either '1' -> (small fixed window) or '2' -> large variable window
///
/// @tparam     T            Output data type. Filtered values are cast to T.
///
/// @todo       Review implementation, improve documentation.
///
template <typename T >
std::vector< T > mean_filter(const std::vector< T >& ref, const std::vector< T >& input, size_t filter_type) {
    std::vector< T > filtered(0);
    mean_filter(ref, input, filter_type, filtered);
    return filtered;
}

//------------------------------------------------------------------------------
/// @brief      Sorts *input* data & returns the sorted indexes.
///             Wraps around std::sort();
///
/// @param[in]  input  Input data to sort
/// @param[out] idx    Output container for sorted indexes.
///
/// @tparam     T       Input data type.
///
template <typename T >
void sort(const std::vector< T >& input, std::vector< size_t >& idx) {
    idx.assign(input.size(), 0);
    std::iota(idx.begin(), idx.end(), 0);
    // sort indexes based on comparing values in v
    std::sort(idx.begin(), idx.end(), [ &input ](size_t i1, size_t i2) { return input[i1] < input[i2]; });
}

//------------------------------------------------------------------------------
/// @brief      Sorts *input* data & returns the sorted indexes (inline initialization overload).
///             Wraps around std::sort();
///
/// @param[in]  input  Input data to sort
///
/// @tparam     T       Input data type.
///
template < typename T >
std::vector< size_t > sort(const std::vector< T >& input) {
    std::vector< size_t > idx(input.size());
    sort(input, idx);
    return idx;
}


template < typename _t >
void ref_resample(const std::vector<_t>& _inputRefs, const std::vector<_t>& _inputVals, const std::vector<_t>& _outputRefs, bool _periodic, std::vector<_t>& _outputVals){
    assert(_inputRefs.size()==_inputVals.size());
    int prev_size=_inputVals.size();
    int new_size=_outputRefs.size();
    if (new_size>1 && prev_size>0 && _inputRefs.size()==prev_size) {
        _outputVals.assign(new_size,0.0);
        float step=prev_size/new_size;

        int j = 0;
        for (int i = 0; i < new_size; ++i){
            float slope = 0.0;
            // int j = 0; // assumes input values sorted by reference, in order to speed up
            while (_inputRefs[j]<_outputRefs[i] && j<prev_size) j++;

            // if (j>0) printf("%d : %d\t%f : %f (%f) -> ",i,j,_outputRefs[i],_inputRefs[j],_inputRefs[j-1]);
            // else printf("%d : %d\t%f : %f -> ",i,j,_outputRefs[i],_inputRefs[j]);

            if (_outputRefs[i]==_inputRefs[j]) {
                // exact match between old and new reference - value is copied
                _outputVals[i]=_inputVals[j];
                // printf("1\n");
                continue;
            }
            else if (j == prev_size) {
                // all input references are smaller than desired output reference
                // last 2 input values are used to compute slope
                if (_periodic){
                    slope = (_inputVals[0]-_inputVals[prev_size-1])/(_inputRefs[0]-_inputRefs[prev_size-1]);
                } else {
                    slope = (_inputVals[prev_size-1]-_inputVals[prev_size-2])/(_inputRefs[prev_size-1]-_inputRefs[prev_size-2]);
                }
                // compute & populate new values
                _outputVals[i] = _inputVals.back() + slope * (_outputRefs[i]-_inputRefs.back());
                // printf("2\n");
            }
            else if (j == 0) {
                // all input references are greater than the desired output reference
                // first 2 input values are used to compute slope
                if (_periodic){
                    slope = (_inputVals[0]-_inputVals[prev_size-1])/(_inputRefs[0]-_inputRefs[prev_size-1]);
                } else {
                    slope = (_inputVals[1]-_inputVals[0])/(_inputRefs[1]-_inputRefs[0]);
                }
                // compute & populate new values
                _outputVals[i] = _inputVals[0] + slope * (_outputRefs[i]-_inputRefs[0]);
                // printf("3\n");
            }
            else {
                // output reference falls between 2 input references
                slope = (_inputVals[j]-_inputVals[j-1])/(_inputRefs[j]-_inputRefs[j-1]);
                // compute & populate new values
                _outputVals[i] = _inputVals[j-1] + slope * (_outputRefs[i]-_inputRefs[j-1]);
                // printf("4\n");
            }
        }
    }
}


template <typename _t>
std::vector<_t> ref_resample(const std::vector<_t>& _inputRefs, const std::vector<_t>& _inputVals, const std::vector<_t>& _outputRefs, bool _periodic){
    std::vector<_t> out;
    ref_resample(_inputRefs, _inputVals, _outputRefs, _periodic, out);
    return out;
}


//------------------------------------------------------------------------------
/// @brief      Computes an interpolated curve from a reference signal.
///
/// @param[in]  x           Input x-values.
/// @param[in]  y           Input y-values.
/// @param[in]  ref_x       Reference x (target x-values).
/// @param[in]  filtering   Filtering flag. If true, output signal is smoothed w/ a mean filter.
/// @param[out] out         Output matrix/container object.
///
/// @tparam     T      Output data type. Interpolated values cast to T.
///
/// @note       Function is inneficient, needs rewrite.
///
/// @note       Results are very dependent on the quality of input data (periodicy).
///
template < typename T >
void interpolate(const std::vector< T >& x, const std::vector< T >& y, const std::vector< T >& ref_x, bool filtering, std::vector< T >& out) {
    // 1. sort vectors (by phase)
    std::vector< T > sorted_x(x.size(), 0.0);
    std::vector< T > sorted_y(y.size(), 0.0);
    std::vector< size_t > idxs = sort(x);
    for (int i = 0; i < idxs.size(); ++i) {
        sorted_x[i] = x[idxs[i]];
        sorted_y[i] = y[idxs[i]];
    }

    // look for non-numeric values on sorted vectors
    for (int i = 1; i < sorted_x.size(); ++i) {
        if (std::isnan(sorted_x[i])) {
            throw std::runtime_error(std::string(__func__) + "(): NaN values on sorted vector!");
        }
    }
    for (int i = 1; i < sorted_y.size(); ++i) {
        if (std::isnan(sorted_y[i])) {
            throw std::runtime_error(std::string(__func__) + "(): NaN values on sorted vector!");
        }
    }

    // 2. smoth output data (optional) to ensure all samples contribute to final solution
    // method 2 should be used if there is chance of value repetition
    std::vector< T > filtered_y(sorted_y.size(), 0.0);
    if (filtering) {
        mean_filter(sorted_x, sorted_y, /* smoothing method */ 2, filtered_y);
    } else {
        filtered_y = sorted_y;
    }

    // look for non-numeric values on filtered vector
    for (int i = 1; i < filtered_y.size(); ++i) {
        if (std::isnan(filtered_y[i])) {
            throw std::runtime_error(std::string(__func__) + "(): NaN values on filtered vector!");
        }
    }

    // 3. resample dataset
    ref_resample(sorted_x, filtered_y, ref_x, true, out);

    // look for non-numeric values on sorted vectors
    for (int i = 1; i < out.size(); ++i) {
        if (std::isnan(out[i])) {
            printf("NaN values on resampled vector ry\n");
        }
    }
}

//------------------------------------------------------------------------------
/// @brief      Computes an interpolated curve from a reference signal (inline initialization overload).
///
/// @param[in]  x           Input x-values.
/// @param[in]  y           Input y-values.
/// @param[in]  ref_x       Reference x (target x-values).
/// @param[in]  filtering   Filtering flag. If true, output signal is smoothed w/ a mean filter.
///
/// @tparam     T      Output data type. Interpolated values cast to T.
///
template < typename  T >
std::vector< T > interpolate(const std::vector< T >& x, const std::vector< T >& y, const std::vector< T >& ref_x, bool filtering) {
    std::vector< T > interpolated(0);
    interpolate(x, y, ref_x, filtering, interpolated);
    return interpolated;
}

}  // namespace math

namespace extra {

//------------------------------------------------------------------------------
/// @brief      Output stream modifier turning text output to boldface.
///
/// @param      os    Output stream
///
/// @return     Modified output stream - further stream operations *with* bold text.
///
inline std::ostream& bold_on(std::ostream& os) {
    return os << "\e[1m";
}


//------------------------------------------------------------------------------
/// @brief      Output stream modifier disabling boldface output.
///
/// @param      os    Output stream
///
/// @return     Modified output stream - further stream operations *without* bold text.
///
inline std::ostream& bold_off(std::ostream& os) {
    return os << "\e[0m";
}

}  // namespace extra

#endif  // MPLEARN_INCLUDE_ETC_UTILS_HPP_
