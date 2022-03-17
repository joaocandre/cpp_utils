#ifndef VOLUME_HPP
#define VOLUME_HPP
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <vector>
#include <iostream>
#include <utility>
#include <random>
#include <ctime>
// #include "matrix.hpp"
#include <storage/subset.hpp>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper classes
namespace std {
template <typename T> class volume;  // forward declaration required for 'using' typedefs below
template<typename dT>
using volume_subset 		= storage::st_subset_base< volume<dT> >;
template<typename dT>
using volume_subset_const 	= storage::st_subset_base< const volume<dT> >;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T> 
class volume {

protected:
	int nLayers,nRows,nCols;
	vector<T> data;

public:
	// value type typedef
	typedef T value_type; // alternative: typename std::remove_reference_t<decltype(std::declval<std::volume<T>>()[0])>

	// basic constructors 
	volume();						   	
	volume(int _layers, int _rows, int _cols);
	volume(int _layers, int _rows, int _cols, const T& _val); // allows implicitely cast types

	// templated constructors
	template <typename iT> volume(const vector<vector<vector<iT>>>& _in);
	template <typename iT, typename = typename enable_if<is_generic_container<iT>()>::type> volume(int _rows, int _cols, int _layers, const iT& _in);  	// from generic container (e.g. volume, volume subset, vector, etc)

	// destructor
	~volume();	

	// indirection operator
	T& operator*();

	// assignment operator overload 					
	template <typename iT> volume<T>& operator=(const iT& _in);

	// element-wise accessors/modifiers (operator () overloads)
	T& 			operator()(int _l, int _r, int _c); 
	const T& 	operator()(int _l, int _r, int _c) const; 
	T& 			operator[](int _id);     
	const T& 	operator[](int _id) const; 

	// bulk accessors/modifiers (operator [] overloads)
	volume_subset<T> 	   operator[](const vector<int>& _idx);
	volume_subset_const<T> operator[](const vector<int>& _idx) const;
	volume_subset<T> 	   operator[](volume_subset<T>&& _sbst);  
	volume_subset_const<T> operator[](volume_subset<T>&& _sbst) const; 

	// access to inner vector object (read-only)
	const vector<T>& data_vector() 	const;

	// size assessment
	size_t rows() 	const;
	size_t cols() 	const;
	size_t layers() const;
	size_t size() 	const;
	vector<size_t> dim() 	  const;
	vector<size_t> pos(int i) const;

	// content assessment 
	bool isCubic()  const;
	bool isEmpty() 	const; 

	// segment index identifiers
	vector<int> allID() 	  			const; 	   
	vector<int> rowID(int _l, int _r) 	const;  
	vector<int> colID(int _l, int _c) 	const;  
	vector<int> towID(int _r, int _c) 	const;  
	vector<int> diagID(int _l) 			const; 
	vector<int> layerID(int _l) 		const;  
	vector<int> rowLayerID(int _r) 		const;  
	vector<int> colLayerID(int _c) 		const;  
	vector<int> layerBlockID(int _l, int _first_row, int _last_row, int _first_col, int _last_col) 	const;  
	vector<int> rowBlockID(int _r, int _first_lay, int _last_lay, int _first_col, int _last_col) 	const;  
	vector<int> colBlockID(int _c, int _first_lay, int _last_lay, int _first_row, int _last_row) 	const;  
	vector<int> cubeID(int _first_lay, int _last_lay, int _first_row, int _last_row, int _first_col, int _last_col) const;  

	//segment acessors/modifiers
	volume_subset<T> 		all();
	volume_subset_const<T>	all() const;
	volume_subset<T> 		row(int _l, int _r);
	volume_subset_const<T>	row(int _l, int _r) const;
	volume_subset<T> 		col(int _l, int _c);
	volume_subset_const<T>	col(int _l, int _c) const;
	volume_subset<T> 		tow(int _r, int _c);
	volume_subset_const<T>	tow(int _r, int _c) const;
	volume_subset<T> 		diag(int _l);
	volume_subset_const<T>	diag(int _l) const;
	volume_subset<T> 		layer(int _l);
	volume_subset_const<T>	layer(int _l) const;
	volume_subset<T> 		rowLayer(int _r);
	volume_subset_const<T>	rowLayer(int _r) const;
	volume_subset<T> 		colLayer(int _c);
	volume_subset_const<T>	colLayer(int _c) const;
	volume_subset<T> 		layerBlock(int _l, int _first_row, int _last_row, int _first_col, int _last_col);
	volume_subset_const<T>	layerBlock(int _l, int _first_row, int _last_row, int _first_col, int _last_col) const;
	volume_subset<T> 		rowBlock(int _r, int _first_lay, int _last_lay, int _first_col, int _last_col);
	volume_subset_const<T>	rowBlock(int _r, int _first_lay, int _last_lay, int _first_col, int _last_col) const;
	volume_subset<T> 		colBlock(int _c, int _first_lay, int _last_lay, int _first_row, int _last_row);
	volume_subset_const<T>	colBlock(int _c, int _first_lay, int _last_lay, int _first_row, int _last_row) const;
	volume_subset<T> 		cube(int _first_lay, int _last_lay, int _first_row, int _last_row, int _first_col, int _last_col);
	volume_subset_const<T>	cube(int _first_lay, int _last_lay, int _first_row, int _last_row, int _first_col, int _last_col) const;

	// template <typename iT, typename = typename conditional<is_nd_container<iT>()>::type> void pushRow(const iT& _in);

	// bulk data modifiers 
	template <typename iT> void setData(const iT& _in);
	template <typename iT, typename=typename enable_if< is_nd_container<iT>()>::type> void pushRow(const iT& _in);
	template <typename iT, typename=typename enable_if<!is_nd_container<iT>()>::type, typename = void> void pushRow(const iT& _in);
	template <typename iT, typename=typename enable_if< is_nd_container<iT>()>::type> void pushCol(const iT& _in);
	template <typename iT, typename=typename enable_if<!is_nd_container<iT>()>::type, typename = void> void pushCol(const iT& _in);
	template <typename iT, typename=typename enable_if< is_nd_container<iT>()>::type> void pushLayer(const iT& _in);
	template <typename iT, typename=typename enable_if<!is_nd_container<iT>()>::type, typename = void> void pushLayer(const iT& _in);
	void popRow();
	void popCol();
	void popLayer();
	void deleteRow(int _r);
	void deleteCol(int _c);
	void deleteLayer(int _l);
	void reshape(int _new_layers, int _new_rows, int _new_cols);
	void resize(int _new_rows, int _new_cols, int _new_layers);
	void clear();	

	//iterator members (advanced vector access, and range-based for (:) loops)
	//wrappers for STL std::vector iterators and const_iterators
	typename vector<T>::iterator begin();
	typename vector<T>::iterator end();
	typename vector<T>::const_iterator begin() const;
	typename vector<T>::const_iterator end()   const;

	//basic cast operator 
	template <typename oT> operator const volume<oT>() const;
	template <typename oT> operator volume<oT>();
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
template <typename T> 
volume<T>::volume() { 
	data  = vector<T> (0); 
	nRows = 0;
	nCols = 0;
	nLayers = 0;
	data.reserve(10*10*10); //assumed required vector size for faster resizing and pushing 
};
template <typename T> 
volume<T>::volume(int _layers, int _rows, int _cols) {
	assert(_rows>0 && _cols>0 && _layers>0);
	for (int i = 0; i < _rows*_cols*_layers; ++i) data.push_back(static_cast<T> (NULL));
	nRows=_rows;
	nCols=_cols;
	nLayers=_layers;
	data.reserve(2*nRows*nCols*nLayers); 
};
template <typename T> 
volume<T>::volume(int _layers, int _rows, int _cols, const T& _val){
	assert(_rows>0 && _cols>0 && _layers>0);
	for (int i = 0; i < _rows*_cols*_layers; ++i) data.push_back(_val);
	nRows=_rows;
	nCols=_cols;
	nLayers=_layers;
	data.reserve(2*nRows*nCols*nLayers); 
};
template <typename T> 
template <typename iT> 
volume<T>::volume(const vector<vector<vector<iT>>>& _in){
	nLayers = _in.size();
	nRows   = 0;
	nCols 	= 0;
	for (int l = 0; l < nLayers; ++l) {
		if (_in[l].size() > nRows) nRows = _in[l].size();
		for (int r = 0; r < nRows; ++r)	{
			if (_in[l][r].size() > nCols) nCols = _in[l][r].size();
		}
	}
	data  = vector<T> (nLayers*nRows*nCols); 
	for (int l = 0; l < nLayers; ++l){
		for (int r = 0; r < nRows; ++r) {
			for (int c = 0; c < nCols; ++c) if (c<_in[l][r].size()) (*this)(l,r,c) = static_cast<T>(_in[l][r][c]);
		}
	}
	data.reserve(2*nRows*nCols*nLayers); 
};
template <typename T> 
template <typename iT, typename> 
volume<T>::volume(int _rows, int _cols, int _layers, const iT& _in){ 
	static_assert(is_generic_container<iT>(), "");
	assert(_rows>0 && _cols>0 && _layers>0);
	assert(_in.size() >= _rows*_cols*_layers); 
	nRows 	= _rows;
	nCols 	= _cols;
	nLayers = _layers;
	data  	=  std::vector<T> (0); 
	for (int i = 0; i < _rows*_cols*_layers; ++i) data.push_back(static_cast<T>(_in[i]));
	data.reserve(2*_rows*_cols*_layers);
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DESTRUCTOR
template <typename T> 
volume<T>::~volume(){ 
	/* ... */ 
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INDIRECTION OPERATOR
template <typename T>
T& volume<T>::operator*(){
	assert(data.size() > 0);
	return data[0];
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ASSIGNMENT OPERATOR
template <typename T> 
template <typename iT> 
volume<T>& volume<T>::operator=(const iT& _in){ 
	static_assert(is_generic_container<iT>(), "");
	assert(_in.size() >= data.size());
	for (int i = 0; i < data.size(); ++i) data[i] = static_cast<T>(_in[i]); 
	return *this;	
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ELEMENT ACCESSORS AND MODIFIERS
template <typename T> 
T& volume<T>::operator()(int _l, int _r, int _c) {
	assert(_l>=0 && _l<nLayers && _r>=0 && _r<nRows && _c>=0 && _c<nCols);
 	return data[_l*(nRows*nCols)+_r*nCols+_c];
};
template <typename T> 
const T& volume<T>::operator()(int _l, int _r, int _c) const {
	assert(_l>=0 && _l<nLayers && _r>=0 && _r<nRows && _c>=0 && _c<nCols);
 	return data[_l*(nRows*nCols)+_r*nCols+_c];
};
template <typename T> 
T& volume<T>::operator[](int _id) {
	assert(_id < data.size()); 
 	return data[_id];
};
template <typename T> 
const T& volume<T>::operator[](int _id) const {
	assert(_id < data.size()); 
 	return data[_id];
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BULK ACCESSORS AND MODIFIERS
template <typename T>
volume_subset<T> volume<T>::operator[](const std::vector<int>& _idx) {
	return volume_subset<T>(this,_idx); // container_subset constructor asserts index validity
};
template <typename T>
volume_subset_const<T> volume<T>::operator[](const std::vector<int>& _idx) const {
	return volume_subset_const<T>(this,_idx); // container_subset constructor asserts index validity
};
template <typename T>
volume_subset<T> volume<T>::operator[](volume_subset<T>&& _sbst) {
	if (_sbst.source() != this && _sbst.source_size()>size()) assert(_sbst.index().back() < size());
	return volume_subset<T>(_sbst); 
};
template <typename T>
volume_subset_const<T> volume<T>::operator[](volume_subset<T>&& _sbst) const {
	if (_sbst.source() != this && _sbst.source_size()>size()) assert(_sbst.index().back() < size());
	return volume_subset_const<T>(_sbst); 
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INNER DATA VECTOR ACCESSOR
template <typename T> 
const vector<T>& volume<T>::data_vector() const {
	return data;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SIZE ASSESSMENT MEMBERS 
template <typename T> 
size_t volume<T>::rows() const{ 
	return nRows; 
};
template <typename T> 
size_t volume<T>::cols() const{ 
	return nCols; 
};
template <typename T> 
size_t volume<T>::layers() const{ 
	return nLayers; 
};
template <typename T> 
size_t volume<T>::size() const{ 
	return data.size(); 
};
template <typename T> 
vector<size_t> volume<T>::dim() const{ 
	return vector<size_t>({nLayers,nRows,nCols}); 
};
template <typename T> 
vector<size_t> volume<T>::pos(int _i) const{
	vector<size_t> dims = dim();
	vector<size_t> ind(3);
	int comp = _i;
	for (int i = 0; i <= 3; i++){
		int dim_sz = 1;
		for (int j = i+1; j < 3; j++) dim_sz*=dims[j];
		while (comp-dim_sz>=0) { ind[i]++; comp-=dim_sz; }
	}
	return ind;
}; 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPARISON/VALUE ASSESSMENT MEMBERS
template <typename T> 
bool volume<T>::isCubic() const{
	return (nLayers == nRows && nRows == nCols);
};
template <typename T> 
bool volume<T>::isEmpty() const{
	return !(nLayers || nRows || nCols);
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INDEX ACCESSORS
template <typename T> 
vector<int> volume<T>::allID() const {  
	int i = 0;
	vector<int> ids(0);
	for (auto& val : data) ids.push_back(i++);
	return ids;
};
template <typename T> 
vector<int> volume<T>::rowID(int _l, int _r) const {  
	assert(_r>=0 && _r < nRows);
	assert(_l>=0 && _l < nLayers);
	vector<int> ids (0);
	for (int c = 0; c < nCols; c++) ids.push_back(_l*(nRows*nCols) + _r*nCols + c);
	return ids;
};
template <typename T> 
vector<int> volume<T>::colID(int _l, int _c) const {  
	assert(_c>=0 && _c < nCols);
	assert(_l>=0 && _l < nLayers);
	vector<int> ids (0); 
	for (int r = 0; r < nRows; r++) ids.push_back(_l*(nRows*nCols) + r*nCols + _c);
	return ids;
};
template <typename T> 
vector<int> volume<T>::towID(int _r, int _c) const {  
	assert(_r>=0 && _r < nRows);
	assert(_c>=0 && _c < nCols);
	vector<int> ids (0); 
	for (int l = 0; l < nLayers; l++) ids.push_back(l*(nRows*nCols) + _r*nCols + _c);
	return ids;
};
template <typename T> 
vector<int> volume<T>::diagID(int _l) const{ 
	assert(_l>=0 && _l < nLayers);
	vector<int> ids(0); 
	for (int r = 0; r < nRows; r++) ids.push_back(_l*(nRows*nCols) + r*(nCols+1));
	return ids;
}; 
template <typename T> 
vector<int> volume<T>::layerID(int _l) const {  
	assert(_l>=0 && _l < nLayers);
	vector<int> ids(0);
	for (int i = 0; i < (nRows*nCols); i++) ids.push_back(_l*(nRows*nCols) + i);
	return ids;
};
template <typename T> 
vector<int> volume<T>::rowLayerID(int _r) const {  
	assert(_r>=0 && _r < nRows);
	vector<int> ids(0);
	for (int l = 0; l < nLayers; l++) {
		for (int c = 0; c < nCols; c++) ids.push_back(l*(nRows*nCols) + _r*(nCols) + c);
	}
	return ids;
};
template <typename T> 
vector<int> volume<T>::colLayerID(int _c) const {  
	assert(_c>=0 && _c < nCols);
	vector<int> ids(0);
	for (int l = 0; l < nLayers; l++) {
		for (int r = 0; r < nRows; r++) ids.push_back(l*(nRows*nCols) + r*(nCols) + _c);
	}
	return ids;
};
template <typename T> 
vector<int> volume<T>::layerBlockID(int _l, int _first_row, int _last_row, int _first_col, int _last_col) const{  
	assert(_l>=0 && _l < nLayers);
	assert(_first_row>=0 && _first_row<_last_row && _last_row<nRows);
	assert(_first_col>=0 && _first_col<_last_col && _last_col<nCols);
	vector<int> ids(0);
	for (int r = _first_row; r < _last_row; ++r) {
		for (int c = _first_col; c < _last_col; ++c) ids.push_back(_l*(nRows*nCols) + r*nCols + c);
	}
	return ids;
};
template <typename T> 
vector<int> volume<T>::rowBlockID(int _r, int _first_lay, int _last_lay, int _first_col, int _last_col) const{  
	assert(_r>=0 && _r < nRows);
	assert(_first_col>=0 && _first_col<_last_col && _last_col<nCols);
	assert(_first_lay>=0 && _first_lay<_last_lay && _last_lay<nLayers);
	vector<int> ids(0);
	for (int l = _first_lay; l < _last_lay; l++) {
		for (int c = _first_col; c < _last_col; c++) ids.push_back(l*(nRows*nCols) + _r*nCols + c);
	}
	return ids;
};
template <typename T> 
vector<int> volume<T>::colBlockID(int _c, int _first_lay, int _last_lay, int _first_row, int _last_row) const{  
	assert(_c>=0 && _c < nCols);
	assert(_first_row>=0 && _first_row<_last_row && _last_row<nRows);
	assert(_first_lay>=0 && _first_lay<_last_lay && _last_lay<nLayers);
	vector<int> ids(0);
	for (int l = _first_lay; l < _last_lay; l++) {
		for (int r = _first_row; r < _last_row; ++r) ids.push_back(l*(nRows*nCols) + r*nCols + _c);
	}
	return ids;
};
template <typename T> 
vector<int> volume<T>::cubeID(int _first_lay, int _last_lay, int _first_row, int _last_row, int _first_col, int _last_col) const{  
	assert(_first_row>=0 && _first_row<_last_row && _last_row<nRows);
	assert(_first_col>=0 && _first_col<_last_col && _last_col<nCols);
	assert(_first_lay>=0 && _first_lay<_last_lay && _last_lay<nLayers);
	vector<int> ids(0);
	for (int l = _first_lay; l < _last_lay; ++l) {
		for (int r = _first_row; r < _last_row; ++r) {
			for (int c = _first_col; c < _last_col; ++c) ids.push_back(l*(nRows*nCols) + r*nCols + c);
		}
	}
	return ids;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SUBSET COPY ACCESSORS 
template <typename T>
volume_subset<T> volume<T>::all(){
	return volume_subset<T>(this,allID());
}; 
template <typename T>
volume_subset_const<T> volume<T>::all() const{
	return volume_subset_const<T>(this,allID());
};
template <typename T>
volume_subset<T> volume<T>::row(int _l, int _r){
	return volume_subset<T>(this,rowID(_l,_r));
}; 
template <typename T>
volume_subset_const<T> volume<T>::row(int _l, int _r) const{
	return volume_subset_const<T>(this,rowID(_l,_r));
};
template <typename T>
volume_subset<T> volume<T>::col(int _l, int _c){
	return volume_subset<T>(this,colID(_l,_c));
}; 
template <typename T>
volume_subset_const<T> volume<T>::col(int _l, int _c) const{
	return volume_subset_const<T>(this,colID(_l,_c));
};
template <typename T>
volume_subset<T> volume<T>::tow(int _r, int _c){
	return volume_subset<T>(this,towID(_r,_c));
}; 
template <typename T>
volume_subset_const<T> volume<T>::tow(int _r, int _c) const{
	return volume_subset_const<T>(this,towID(_r,_c));
};
template <typename T>
volume_subset<T> volume<T>::diag(int _l){
	return volume_subset<T>(this,diagID(_l));
}; 
template <typename T>
volume_subset_const<T> volume<T>::diag(int _l) const{
	return volume_subset_const<T>(this,diagID(_l));
};
template <typename T>
volume_subset<T> volume<T>::layer(int _l){
	return volume_subset<T>(this,layerID(_l));
}; 
template <typename T>
volume_subset_const<T> volume<T>::layer(int _l) const{
	return volume_subset_const<T>(this,layerID(_l));
};
template <typename T>
volume_subset<T> volume<T>::rowLayer(int _r){
	return volume_subset<T>(this,rowLayerID(_r));
}; 
template <typename T>
volume_subset_const<T> volume<T>::rowLayer(int _r) const{
	return volume_subset_const<T>(this,rowLayerID(_r));
};
template <typename T>
volume_subset<T> volume<T>::colLayer(int _c){
	return volume_subset<T>(this,colLayerID(_c));
}; 
template <typename T>
volume_subset_const<T> volume<T>::colLayer(int _c) const{
	return volume_subset_const<T>(this,colLayerID(_c));
};
template <typename T>
volume_subset<T> volume<T>::layerBlock(int _l, int _first_row, int _last_row, int _first_col, int _last_col){
	return volume_subset<T>(this,layerBlockID(_l,_first_row,_last_row,_first_col,_last_col));
}; 
template <typename T>
volume_subset_const<T> volume<T>::layerBlock(int _l, int _first_row, int _last_row, int _first_col, int _last_col) const{
	return volume_subset_const<T>(this,layerBlockID(_l,_first_row,_last_row,_first_col,_last_col));
};
template <typename T>
volume_subset<T> volume<T>::rowBlock(int _r, int _first_lay, int _last_lay, int _first_col, int _last_col){
	return volume_subset<T>(this,rowBlockID(_r,_first_lay,_last_lay,_first_col,_last_col));
}; 
template <typename T>
volume_subset_const<T> volume<T>::rowBlock(int _r, int _first_lay, int _last_lay, int _first_col, int _last_col) const{
	return volume_subset_const<T>(this,rowBlockID(_r,_first_lay,_last_lay,_first_col,_last_col));
};
template <typename T>
volume_subset<T> volume<T>::colBlock(int _c, int _first_lay, int _last_lay, int _first_row, int _last_row){
	return volume_subset<T>(this,colBlockID(_c,_first_lay,_last_lay,_first_row,_last_row));
}; 
template <typename T>
volume_subset_const<T> volume<T>::colBlock(int _c, int _first_lay, int _last_lay, int _first_row, int _last_row) const{
	return volume_subset_const<T>(this,colBlockID(_c,_first_lay,_last_lay,_first_row,_last_row));
};
template <typename T>
volume_subset<T> volume<T>::cube(int _first_lay, int _last_lay, int _first_row, int _last_row, int _first_col, int _last_col){
	return volume_subset<T>(this,cubeID(_first_lay,_last_lay,_first_row,_last_row,_first_col,_last_col));
}; 
template <typename T>
volume_subset_const<T> volume<T>::cube(int _first_lay, int _last_lay, int _first_row, int _last_row, int _first_col, int _last_col) const{
	return volume_subset_const<T>(this,cubeID(_first_lay,_last_lay,_first_row,_last_row,_first_col,_last_col));
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MODIFIERS
template <typename T>
template <typename iT>  
void volume<T>::setData(const iT& _in) { 
	static_assert(is_generic_container<iT>(), "");
	assert(_in.size() >= data.size());
	for (int i = 0; i < data.size(); ++i) data[i] = _in[i]; // 
};
template <typename T> 
template <typename iT, typename>  
void volume<T>::pushRow(const iT& _in){
	static_assert(is_nd_container<iT>(), "");
	if (!nLayers && !nCols){
		vector<size_t> dims = _in.dim();
		assert (dims.size() == 2);
		nLayers = dims[0];
		nCols 	= dims[1];
	} else assert(_in.size() >= nLayers*nCols);
	for (int l = nLayers-1; l>=0; l--) { 
		// add new row to each layer 
		// storage is row-major, range insert can be used
		data.insert(data.begin()+(l+1)*(nRows*nCols),_in.begin()+l*nCols,_in.begin()+l*nCols+nCols );
	}
	nRows++;
}
template <typename T> 
template <typename iT, typename, typename>  
void volume<T>::pushRow(const iT& _in){
	static_assert(is_generic_container<iT>(), "");
	if (!nLayers && !nCols) {
		nLayers = 1;
		nCols 	= _in.size();
	} else assert(_in.size() >= nLayers*nCols);
	for (int l = nLayers-1; l>=0; l--) { 
		// add new row to each layer
		// storage is row-major, range insert can be used
		data.insert(data.begin()+(l+1)*(nRows*nCols),_in.begin()+l*nCols,_in.begin()+l*nCols+nCols );
	}
	nRows++;
}
template <typename T> 
template <typename iT, typename>  
void volume<T>::pushCol(const iT& _in){
	static_assert(is_nd_container<iT>(), "");
	if (!nLayers && !nRows){
		vector<size_t> dims = _in.dim();
		assert (dims.size() == 2);
		nLayers = dims[0];
		nRows 	= dims[1];
	} 
	assert(_in.size() >= nLayers*nRows);
	for (int l=nLayers-1; l>=0; l--){ //add new col to each layer
		for (int r=nRows-1; r>=0; r--) data.insert(data.begin()+(l*(nRows*nCols) + r*nCols + (nCols)),_in[r*nLayers+l]); //bulk insert possible?
	}
	nCols++;
}
template <typename T> 
template <typename iT, typename,typename>  
void volume<T>::pushCol(const iT& _in){
	static_assert(is_generic_container<iT>(), "");
	if (!nLayers && !nRows){
		nLayers = 1;
		nRows 	= _in.size();
	} 
	assert(_in.size() >= nLayers*nRows);
	for (int l=nLayers-1; l>=0; l--){ //add new col to each layer
		for (int r=nRows-1; r>=0; r--) data.insert(data.begin()+(l*(nRows*nCols) + r*nCols + (nCols)),_in[r*nLayers+l]); //bulk insert possible?
	}
	nCols++;
}
template <typename T> 
template <typename iT, typename>  
void volume<T>::pushLayer(const iT& _in){
	static_assert(is_nd_container<iT>(), "");
	if (!nRows && !nCols){
		vector<size_t> dims = _in.dim();
		assert (dims.size() == 2);
		nRows = dims[0];
		nCols = dims[1];
	}
	assert(_in.size() >= nRows*nCols);
	for (int i = 0; i < nRows*nCols; ++i) data.push_back(_in[i]);
	nLayers++;
}
template <typename T> 
template <typename iT, typename, typename>  
void volume<T>::pushLayer(const iT& _in){
	static_assert(is_generic_container<iT>(), "");
	if (!nRows && !nCols){
		nRows = 1;
		nCols = _in.size();
	}
	assert(_in.size() >= nRows*nCols);
	for (int i = 0; i < nRows*nCols; ++i) data.push_back(_in[i]);
	nLayers++;
}
template <typename T> 
void volume<T>::popRow(){
	assert(nRows > 0);
	if (nRows==1){
		clear();
	} else {
		for (int l = 0; l < nLayers; ++l) {
			for (int c = (nCols-1); c >= 0 ; ++c) data.erase(data.begin()+(l*(nRows*nCols) + (nRows-1)*nCols + c));
		}
	nRows--;
	}
}
template <typename T> 
void volume<T>::popCol(){
	assert(nCols > 0);
	if (nCols==1){
		clear();
	} else {
		for (int l = 0; l < nLayers; ++l){
			for (int r = (nRows-1); r >= 0; ++r) data.erase(data.begin()+(l*(nRows*nCols) + r*nCols + (nCols-1)));
		}
		nCols--;
	}	
}
template <typename T> 
void volume<T>::popLayer(){
	assert(nLayers > 0);
	if (nLayers==1) {
		clear();
	} else {
		data.resize(nRows*nCols*(nLayers-1));
		nLayers--;
	}	
}
template <typename T> 
void volume<T>::deleteRow(int _r){
	assert(_r>=0 && _r < nRows);
	if (nRows==1) {
		clear();
	} else {
		for (int l = 0; l < nLayers; ++l) {
			for (int c = (nCols-1); c >= 0 ; ++c) data.erase(data.begin()+(l*(nRows*nCols) + _r*nCols + c));
		}
		nRows--;
	}	
}
template <typename T> 
void volume<T>::deleteCol(int _c){
	assert(_c>=0 && _c < nCols);
	if (nCols==1) {
		clear();
	} else {
		for (int l = 0; l < nLayers; ++l){
			for (int r = (nRows-1); r >= 0; ++r) data.erase(data.begin()+(l*(nRows*nCols) + r*nCols + _c));
		}
		nCols--;	
	}	
}
template <typename T> 
void volume<T>::deleteLayer(int _l){
	assert(_l>=0 && _l < nLayers);
	if (nLayers==1) {
		clear();
	} else {
		data.erase(data.begin()+_l*(nRows*nCols),data.begin()+(_l+1)*(nRows*nCols));
		nLayers--;
	}	
}
template <typename T> 
void volume<T>::reshape(int _new_layers, int _new_rows, int _new_cols){
	assert(_new_rows>=0 && _new_cols>=0 && _new_layers>=0);
	if (!_new_rows || !_new_cols || !_new_layers) clear(); 
	//match cols
	while (nCols>_new_cols) popCol();
	while (nCols<_new_cols) pushCol(vector<T> (nLayers*nRows, static_cast<T> (NULL)));
	// printf("matched cols %lu -> %lu [%lu]\n",nCols,_new_cols,data.size());
	//match rows
	while (nRows>_new_rows) popRow();
	while (nRows<_new_rows) pushRow(vector<T> (nLayers*nCols, static_cast<T> (NULL)));
	// printf("matched rows %lu -> %lu [%lu,%lu,%lu]\n",nRows,_new_rows,nLayers,nRows,nCols);
	//match layers	static_cast<T> (NULL)
	while (nLayers>_new_layers) popLayer();
	while (nLayers<_new_layers) pushLayer(vector<T> (nRows*nCols, static_cast<T> (NULL)));
	// data.resize(_new_rows*_new_cols*_new_layers);
	// nLayers=_new_layers;
};
template <typename T> 
void volume<T>::resize(int _new_rows, int _new_cols, int _new_layers){
	if (!_new_layers || !_new_rows || !_new_cols) clear();
	if (_new_layers<1 || _new_rows<1 || _new_cols<1) throw std::string("INVALID INPUT ARGUMENT");
	if (_new_rows == nRows && _new_cols == nCols){
		//directly resize data vector
		if (_new_layers>nLayers) for (int i = nLayers*nRows*nCols; i < _new_layers*nRows*nCols; ++i) data.push_back(0);
		if (_new_layers<nLayers) data.resize(_new_layers*nRows*nCols);
		nLayers=_new_layers;
	} else {
		std::vector<T> tmp_data = data; //(_new_rows*_new_cols);
		data.resize(_new_layers*_new_rows*_new_cols);
		for (int l = 0; l< _new_layers; l++) {
			for (int r = 0; r < _new_rows; ++r)	{
				for (int c = 0; c < _new_cols; ++c) {
					if (l<nLayers && r<nRows && c<nCols) data[l*(_new_rows*_new_cols)+r*_new_cols+c]=(*this)(l,r,c);
					else data[l*(_new_rows*_new_cols)+r*_new_cols+c] = 0.0;
				}
			}
		}
		nLayers=_new_layers;
		nRows=_new_rows;
		nCols=_new_cols;
	}
};
template <typename T> 
void volume<T>::clear() {
	data.resize(0);
	nRows = 0;
	nCols = 0;
	nLayers = 0;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ITERATORS
template <typename T> 
typename vector<T>::iterator volume<T>::begin() {
	return data.begin();
};
template <typename T> 
typename vector<T>::iterator volume<T>::end() {
	return data.end();
};
template <typename T> 
typename vector<T>::const_iterator volume<T>::begin() const {
	return data.begin();
};
template <typename T> 
typename vector<T>::const_iterator volume<T>::end() const {
	return data.end();
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CAST OPERATOR OVERLOADS
template <typename T> 
template <typename oT>
volume<T>::operator volume<oT>(){
	return volume<oT> (nRows,nCols,nLayers,std::vector<oT>(data.begin(),data.end()));  
};
template <typename T> 
template <typename oT>
volume<T>::operator const volume<oT>() const {
	return volume<oT> (nRows,nCols,nLayers,std::vector<oT>(data.begin(),data.end()));  
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPARISON OPERATORS (NON-MEMBERS)
// negation(!) operator is defined by default
template <typename T, typename _inputT> 
inline bool operator==(const volume<T>& _vol, const volume<_inputT>& _input_vol){ 
	if (_vol.rows() != _input_vol.rows() || _vol.cols() != _input_vol.cols()) return false;
	for (int r = 0; r < _vol.rows(); ++r){
		for (int c=0; c < _vol.cols(); ++c){
			for (int l=0; l < _vol.layers(); ++l){
				if (_vol(l,r,c)!=_input_vol(l,r,c)) return false;
			}
		}	
	}
	return true;
};
template <typename T, typename	 _inputT> 
inline bool operator!=(const volume<T>& _vol, const volume<_inputT>& _input_vol){
	return !operator==(_vol,_input_vol);
};
template <typename T, typename _inputT> 
inline bool operator< (const volume<T>& _vol, const volume<_inputT>& _input_vol){ 
	return (_vol.length() < _input_vol.length());
};
template <typename T, typename _inputT> 
inline bool operator> (const volume<T>& _vol, const volume<_inputT>& _input_vol){
	return  operator< (_input_vol,_vol);
};
template <typename T, typename _inputT> 
inline bool operator<=(const volume<T>& _vol, const volume<_inputT>& _input_vol){
	return !operator> (_vol,_input_vol);
};
template <typename T, typename _inputT> 
inline bool operator>=(const volume<T>& _vol, const volume<_inputT>& _input_vol){
	return !operator< (_vol,_input_vol);
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace std
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif