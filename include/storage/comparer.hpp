#ifndef COMPARER_HPP
#define COMPARER_HPP
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <functional>
#include <vector>
#include <storage/type_check.hpp>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Provides a comparer class for generic storage containers, requiring only:
// 1) size_t size() [for runtime assertions];
// 2) operator[](int) [for data retrieval];
// 3) boolean function cpr defined for the type;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace std {
namespace storage {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// _lDT, _lCT : left_operand data type && container type; _rDT: right_operand data type;
// template <typename _lDT, typename _lCT, typename _rDT>
// class st_bulk_comparer_base {
// protected:
// 	const _lCT* left_operand;
// 	const _rDT* right_operand;
// 	std::function<bool(_lDT,_rDT)> cpr;
// public:
// 	st_bulk_comparer_base(const _lCT* _in_lhs, const _rDT* _in_rhs, const std::function<bool(_lDT,_rDT)>& _cpr) : left_operand(_in_lhs), right_operand(_in_rhs), cpr(_cpr) { /* */ };
// 	explicit operator bool() const { 
// 		if (!(*left_operand).size()) return false;
// 		for (int i = 0; i < (*left_operand).size(); ++i) if (!cpr((*left_operand)[i],(*right_operand))) return false;
// 		return true;
// 	};
// 	operator std::vector<bool> () const {
// 		std::vector<bool> mask ((*left_operand).size());
// 		for (int i = 0; i < mask.size(); ++i) mask[i]=cpr((*left_operand)[i],(*right_operand));
// 		return mask;
// 	};
// };
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // _lDT, _lCT : left_operand data type && container type; _rDT, _rCT : right_operand data type && container type;
// template <typename _lDT, typename _lCT, typename _rDT,  typename _rCT>
// class st_elementwise_comparer_base {
// protected:
// 	const _lCT* left_operand;
// 	const _rCT* right_operand;
// 	std::function<bool(_lDT,_rDT)> cpr;
// public:
// 	st_elementwise_comparer_base(_lCT* _in_lhs, _rCT* _in_rhs, const std::function<bool(_lDT,_rDT)>& _cpr) : left_operand(_in_lhs), right_operand(_in_rhs), cpr(_cpr) { /* */ };
// 	explicit operator bool() const { 
// 		if (!(*left_operand).size()) return false; // in the case of empty containers
// 		int len = min((*left_operand).size(),(*right_operand).size());
// 		for (int i = 0; i < len; ++i) if (!cpr((*left_operand)[i],(*right_operand)[i])) return false; // will only return true if all elements of the smallest operand verity the condition
// 		return true;
// 	};
// 	operator std::vector<bool> () const {
// 		std::vector<bool> mask (min((*left_operand).size(),(*right_operand).size()),false);
// 		for (int i = 0; i < mask.size(); ++i) mask[i]=cpr((*left_operand)[i],(*right_operand)[i]);
// 		return mask;
// 	};
// };	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// performs comparison on container-type classes with other containers (elementwise)
template <typename lT, typename rT>
class st_comparer_base {
protected:
	// container operand pointers
	const lT* left_operand;
	const rT* right_operand;
	// type descriptors
	typedef typename lT::value_type ltype;
	typedef typename rT::value_type rtype;
	// comparison function
	std::function<bool(ltype,rtype)> cpr;
public:
	st_comparer_base(lT* _in_lhs, rT* _in_rhs, const std::function<bool(ltype,rtype)>& _cpr) : left_operand(_in_lhs), right_operand(_in_rhs), cpr(_cpr) { 
		static_assert(is_generic_container<lT>(), "");
		static_assert(is_generic_container<rT>(), "");
	};
	explicit operator bool() const { 
		if (!(*left_operand).size()) return false; // in the case of empty containers
		int len = min((*left_operand).size(),(*right_operand).size());
		for (int i = 0; i < len; ++i) if (!cpr((*left_operand)[i],(*right_operand)[i])) return false;
		return true;
	};
	operator std::vector<bool> () const { //cast to vector<int> (bool mask)
		std::vector<bool> mask (min((*left_operand).size(),(*right_operand).size()),false);
		for (int i = 0; i < mask.size(); ++i) mask[i]=cpr((*left_operand)[i],(*right_operand)[i]);
		return mask;
	};
	operator std::vector<int> () const { //cast to vector<int> (index mask)
		std::vector<int> ids (0);
		for (int i = 0; i < min((*left_operand).size(),(*right_operand).size()); ++i) if (cpr((*left_operand)[i],(*right_operand)[i])) ids.push_back((*left_operand).index()[i]);
		return ids;
	};
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// performs comparison on container-type classes with single object
template <typename lT, typename rT>
class st_bulk_comparer_base {
protected:
	// container operand pointers
	const lT* left_operand;
	const rT* right_operand;
	// type descriptors
	typedef typename lT::value_type ltype;
	typedef rT				rtype;
	// comparison function
	std::function<bool(ltype,rtype)> cpr;
public:
	st_bulk_comparer_base(lT* _in_lhs, rT* _in_rhs, const std::function<bool(ltype,rtype)>& _cpr) : left_operand(_in_lhs), right_operand(_in_rhs), cpr(_cpr) { 
		static_assert(is_generic_container<lT>(), "");
	};
	explicit operator bool() const { 
		if (!(*left_operand).size()) return false;  // in the case of empty containers
		for (int i = 0; i < (*left_operand).size(); ++i) if (!cpr((*left_operand)[i],(*right_operand))) return false;
		return true;
	};
	operator std::vector<bool> () const { 			//cast to vector<int> (bool mask)
		std::vector<bool> mask ((*left_operand).size(),false);
		for (int i = 0; i < mask.size(); ++i) mask[i]=cpr((*left_operand)[i],(*right_operand));
		return mask;
	};
	operator std::vector<int> () const { 			//cast to vector<int> (index mask)
		std::vector<int> ids (0);
		for (int i = 0; i < (*left_operand).size(); ++i) if (cpr((*left_operand)[i],(*right_operand))) ids.push_back((*left_operand).index()[i]);
		return ids;
	};

};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // storage
} // std
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif