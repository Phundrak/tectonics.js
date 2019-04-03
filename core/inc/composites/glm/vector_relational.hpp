#pragma once

#include <valarray>

#include <glm/vec3.hpp>    	// vec2, bvec2, dvec2, ivec2 and uvec2

#include "../many.hpp"

namespace composites
{
	using namespace glm;

	// template<length_t L, typename T, qualifier Q>
	// void greaterThan(const std::valarray<vec<L,T,Q>>& a, const T b, std::valarray<vec<L,bool,defaultp>>& out)
	// {
	// 	for (unsigned int i = 0; i < a.size(); ++i)
	// 	{
	// 		out[i] = glm::greaterThan(a[i], b);
	// 	}
	// }
	// template<length_t L, typename T, qualifier Q>
	// void greaterThanEqual(const std::valarray<vec<L,T,Q>>& a, const T b, std::valarray<vec<L,bool,defaultp>>& out)
	// {
	// 	for (unsigned int i = 0; i < a.size(); ++i)
	// 	{
	// 		out[i] = glm::greaterThanEqual(a[i], b);
	// 	}
	// }
	// template<length_t L, typename T, qualifier Q>
	// void lessThan(const std::valarray<vec<L,T,Q>>& a, const T b, std::valarray<vec<L,bool,defaultp>>& out)
	// {
	// 	for (unsigned int i = 0; i < a.size(); ++i)
	// 	{
	// 		out[i] = glm::lessThan(a[i], b);
	// 	}
	// }
	// template<length_t L, typename T, qualifier Q>
	// void lessThanEqual(const std::valarray<vec<L,T,Q>>& a, const T b, std::valarray<vec<L,bool,defaultp>>& out)
	// {
	// 	for (unsigned int i = 0; i < a.size(); ++i)
	// 	{
	// 		out[i] = glm::lessThanEqual(a[i], b);
	// 	}
	// }





	// template<length_t L, typename T, qualifier Q>
	// void greaterThan(const std::valarray<vec<L,T,Q>>& a, const std::valarray<T> b, std::valarray<vec<L,bool,defaultp>>& out)
	// {
	// 	for (unsigned int i = 0; i < a.size(); ++i)
	// 	{
	// 		out[i] = glm::greaterThan(a[i], b[i]);
	// 	}
	// }
	// template<length_t L, typename T, qualifier Q>
	// void greaterThanEqual(const std::valarray<vec<L,T,Q>>& a, const std::valarray<T> b, std::valarray<vec<L,bool,defaultp>>& out)
	// {
	// 	for (unsigned int i = 0; i < a.size(); ++i)
	// 	{
	// 		out[i] = glm::greaterThanEqual(a[i], b[i]);
	// 	}
	// }
	// template<length_t L, typename T, qualifier Q>
	// void lessThan(const std::valarray<vec<L,T,Q>>& a, const std::valarray<T> b, std::valarray<vec<L,bool,defaultp>>& out)
	// {
	// 	for (unsigned int i = 0; i < a.size(); ++i)
	// 	{
	// 		out[i] = glm::lessThan(a[i], b[i]);
	// 	}
	// }
	// template<length_t L, typename T, qualifier Q>
	// void lessThanEqual(const std::valarray<vec<L,T,Q>>& a, const std::valarray<T> b, std::valarray<vec<L,bool,defaultp>>& out)
	// {
	// 	for (unsigned int i = 0; i < a.size(); ++i)
	// 	{
	// 		out[i] = glm::lessThanEqual(a[i], b[i]);
	// 	}
	// }




	template<length_t L, typename T, qualifier Q>
	void greaterThan(const std::valarray<vec<L,T,Q>>& a, const vec<L,T,Q> b, std::valarray<vec<L,bool,defaultp>>& out)
	{
		for (unsigned int i = 0; i < a.size(); ++i)
		{
			out[i] = glm::greaterThan(a[i], b);
		}
	}
	template<length_t L, typename T, qualifier Q>
	void greaterThanEqual(const std::valarray<vec<L,T,Q>>& a, const vec<L,T,Q> b, std::valarray<vec<L,bool,defaultp>>& out)
	{
		for (unsigned int i = 0; i < a.size(); ++i)
		{
			out[i] = glm::greaterThanEqual(a[i], b);
		}
	}
	template<length_t L, typename T, qualifier Q>
	void lessThan(const std::valarray<vec<L,T,Q>>& a, const vec<L,T,Q> b, std::valarray<vec<L,bool,defaultp>>& out)
	{
		for (unsigned int i = 0; i < a.size(); ++i)
		{
			out[i] = glm::lessThan(a[i], b);
		}
	}
	template<length_t L, typename T, qualifier Q>
	void lessThanEqual(const std::valarray<vec<L,T,Q>>& a, const vec<L,T,Q> b, std::valarray<vec<L,bool,defaultp>>& out)
	{
		for (unsigned int i = 0; i < a.size(); ++i)
		{
			out[i] = glm::lessThanEqual(a[i], b);
		}
	}







	template<length_t L, typename T, qualifier Q>
	void greaterThan(const std::valarray<vec<L,T,Q>>& a, const std::valarray<vec<L,T,Q>>& b, std::valarray<vec<L,bool,defaultp>>& out)
	{
		for (unsigned int i = 0; i < a.size(); ++i)
		{
			out[i] = glm::greaterThan(a[i], b[i]);
		}
	}
	template<length_t L, typename T, qualifier Q>
	void greaterThanEqual(const std::valarray<vec<L,T,Q>>& a, const std::valarray<vec<L,T,Q>>& b, std::valarray<vec<L,bool,defaultp>>& out)
	{
		for (unsigned int i = 0; i < a.size(); ++i)
		{
			out[i] = glm::greaterThanEqual(a[i], b[i]);
		}
	}
	template<length_t L, typename T, qualifier Q>
	void lessThan(const std::valarray<vec<L,T,Q>>& a, const std::valarray<vec<L,T,Q>>& b, std::valarray<vec<L,bool,defaultp>>& out)
	{
		for (unsigned int i = 0; i < a.size(); ++i)
		{
			out[i] = glm::lessThan(a[i], b[i]);
		}
	}
	template<length_t L, typename T, qualifier Q>
	void lessThanEqual(const std::valarray<vec<L,T,Q>>& a, const std::valarray<vec<L,T,Q>>& b, std::valarray<vec<L,bool,defaultp>>& out)
	{
		for (unsigned int i = 0; i < a.size(); ++i)
		{
			out[i] = glm::lessThanEqual(a[i], b[i]);
		}
	}




}