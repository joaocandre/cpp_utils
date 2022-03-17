#ifndef SORTED_LIBRARY_HPP
#define SORTED_LIBRARY_HPP
//////////////////////////////////////////////////////////////////////////////////////////
#include <vector>
#include <cassert>
//////////////////////////////////////////////////////////////////////////////////////////
namespace std{
//////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
class sortedLibrary {
public:
	typedef unsigned int id_type;
	typedef T value_type;

	sortedLibrary();
	sortedLibrary(size_t size);
	sortedLibrary(size_t size, const T& val);

	void setMaxCapacity(size_t max_elements);

	size_t size() const;
	size_t pos(id_type id) const;

	T& operator[](size_t index);
	const T& operator[](size_t index) const;
	T& operator()(id_type index);
	const T& operator()(id_type index) const;

	void push(const T& entry);
	void del(size_t index);
	void del(id_type id);
	void pop();

	typename vector<T>::iterator begin();
	typename vector<T>::iterator end();
protected:
	vector<T> library;
};
//////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
sortedLibrary<T>::sortedLibrary() {
	// should check if 
	library = vector<T> (0);
	library.reserve(100);
};
template <typename T>
sortedLibrary<T>::sortedLibrary(size_t size) {
	// should check if 
	library = std::vector<T> (size);
	library.reserve(100);
};
template <typename T>
sortedLibrary<T>::sortedLibrary(size_t size, const T& val) {
	// should check if 
	library = std::vector<T> (size,val);
	library.reserve(100);
};
template <typename T>
void sortedLibrary<T>::setMaxCapacity(size_t max_elements) {
	library.reserve(max_elements);
};
template <typename T>
size_t sortedLibrary<T>::size() const {
	return library.size();
}
template <typename T>
size_t sortedLibrary<T>::pos(id_type id) const {
	if (library.size()>0) {
		int pos = 0;
		while (library[pos].id()<id && pos<library.size()) pos++;
		if (pos<library.size() && library[pos].id()==id) return pos;
		else return library.size(); //not found on library
	}
};
template <typename T>
T& sortedLibrary<T>::operator[](size_t index) {
	assert (index < library.size());
	return library[index];
}
template <typename T>
const T& sortedLibrary<T>::operator[](size_t index) const {
	assert (index < library.size());
	return library[index];
}
template <typename T>
T& sortedLibrary<T>::operator()(id_type id) {
	size_t p = pos(id);
	assert (p < library.size());
	return library[p];
}
template <typename T>
const T& sortedLibrary<T>::operator()(id_type id) const {
	size_t p = pos(id);
	assert (p < library.size());
	return library[p];
}
template <typename T>
void sortedLibrary<T>::push(const T& entry){
	assert(library.size()+1 < library.capacity());
	if (library.size()>0) {
		int pos = 0;
		while (library[pos].id()<entry.id() && pos<library.size()-1) pos++;
		library.insert(library.begin()+pos,entry);
	} else library.push_back(entry);
};
template <typename T>
void sortedLibrary<T>::del(size_t index){
	assert(index < library.size());
	library.erase(library.begin()+index);
};
template <typename T>
void sortedLibrary<T>::del(id_type id){
	size_t p = pos(id);
	if (p<library.size()) library.erase(library.begin()+p);
};
template <typename T>
void sortedLibrary<T>::pop(){
	library.pop_back();
};
template <typename T>
typename std::vector<T>::iterator sortedLibrary<T>::begin() {
	return library.begin();
};
template <typename T>
typename std::vector<T>::iterator sortedLibrary<T>::end() {
	return library.end();
};
//////////////////////////////////////////////////////////////////////////////////////////
}; // std
//////////////////////////////////////////////////////////////////////////////////////////
#endif