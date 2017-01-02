#pragma once


#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <windows.h>
#include <mmsystem.h>
#include <process.h>
#include <sstream>
#include <thread>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv/cvaux.h>
#include <opencv2/calib3d.hpp>
#include <opencv2/xfeatures2d.hpp>

using std::string;
using std::set;
using std::vector;
using std::map;

#define RETV(x, y) {if((x)) return (y);}
#define RET(x) {if((x)) return;}


namespace match
{

	struct sParseTree
	{
		int type; // 0=tree, 1=exec
		char str[128];
		int roi[4]; // x,y,w,h
		int depth; // only use parsing
		bool isRelation; // parent relation coordinate roi
		float threshold; // match max threshold, {threshold_0.1 ~ threshold_0.9 ~~ etc}
		int matchType; // 0:templateMatch, 1:featureMatch, {templatematch or featurematch}
		int lineNum; // for error message
		int result;	// for match traverse, only use excute tree, -1:not visit, 0:fail, 1:success
		cv::Point matchLoc; // match location, only use execution
		sParseTree *next;
		sParseTree *child;
	};


	/// Auto Lock, Unlock
	template<class T>
	class AutoLock
	{
	public:
		AutoLock(T& t) : m_t(t) { m_t.Lock(); }
		~AutoLock() { m_t.Unlock(); }
		T &m_t;
	};


	/// Critical Section auto initial and delete
	class CriticalSection
	{
	public:
		CriticalSection();
		~CriticalSection();
		void Lock();
		void Unlock();
	protected:
		CRITICAL_SECTION m_cs;
	};

	inline CriticalSection::CriticalSection() {
		InitializeCriticalSection(&m_cs);
	}
	inline CriticalSection::~CriticalSection() {
		DeleteCriticalSection(&m_cs);
	}
	inline void CriticalSection::Lock() {
		EnterCriticalSection(&m_cs);
	}
	inline void CriticalSection::Unlock() {
		LeaveCriticalSection(&m_cs);
	}

	/// auto critical section lock, unlock
	class AutoCSLock : public AutoLock<CriticalSection>
	{
	public:
		AutoCSLock(CriticalSection &cs) : AutoLock(cs) { }
	};



	template <class Seq>
	void rotateright(Seq &seq)
	{
		if (seq.size() > 1)
			std::rotate(seq.rbegin(), seq.rbegin() + 1, seq.rend());
	}

	// elements를 회전해서 제거한다.
	template <class Seq>
	void rotatepopvector(Seq &seq, const unsigned int idx)
	{
		if ((seq.size() - 1) > idx)
			std::rotate(seq.begin() + idx, seq.begin() + idx + 1, seq.end());
		seq.pop_back();
	}
}


#include "vector3.h"
#include "rectcontour.h"
#include "featurematch.h"



#ifdef _DEBUG
#pragma comment(lib, "opencv_core310d.lib")
#pragma comment(lib, "opencv_imgcodecs310d.lib")
#pragma comment(lib, "opencv_features2d310d.lib")
#pragma comment(lib, "opencv_videoio310d.lib")
#pragma comment(lib, "opencv_highgui310d.lib")
#pragma comment(lib, "opencv_imgproc310d.lib")
#pragma comment(lib, "opencv_flann310d.lib")
#pragma comment(lib, "opencv_xfeatures2d310d.lib")
#pragma comment(lib, "opencv_calib3d310d.lib")
#else
#pragma comment(lib, "opencv_core310.lib")
#pragma comment(lib, "opencv_imgcodecs310.lib")
#pragma comment(lib, "opencv_features2d310.lib")
#pragma comment(lib, "opencv_videoio310.lib")
#pragma comment(lib, "opencv_highgui310.lib")
#pragma comment(lib, "opencv_imgproc310.lib")
#pragma comment(lib, "opencv_flann310.lib")
#pragma comment(lib, "opencv_xfeatures2d310.lib")
#pragma comment(lib, "opencv_calib3d310.lib")
#endif