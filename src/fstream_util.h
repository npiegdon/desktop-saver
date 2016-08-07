// DesktopSaver, (c)2006-2016 Nicholas Piegdon, MIT licensed
#pragma once

//
// NOTE: The following code was taken from:
// http://www.codeproject.com/vcpp/stl/upgradingstlappstounicode.asp
//

#include <locale>

// nb: MSVC6+Stlport can't handle "std::"
// appearing in the NullCodecvtBase typedef.
using std::codecvt; 
typedef codecvt < wchar_t , char , mbstate_t > NullCodecvtBase;

class NullCodecvt
   : public NullCodecvtBase
{
public:
   typedef wchar_t _E;
   typedef char _To;
   typedef mbstate_t _St;

   explicit NullCodecvt( size_t _R=0 ) : NullCodecvtBase(_R) { }

protected:
   virtual result do_in( _St& _State ,
      const _To* _F1 , const _To* _L1 , const _To*& _Mid1 ,
      _E* F2 , _E* _L2 , _E*& _Mid2
      ) const
   {
      return noconv;
   }
   virtual result do_out( _St& _State ,
      const _E* _F1 , const _E* _L1 , const _E*& _Mid1 ,
      _To* F2, _E* _L2 , _To*& _Mid2
      ) const
   {
      return noconv;
   }
   virtual result do_unshift( _St& _State , 
      _To* _F2 , _To* _L2 , _To*& _Mid2 ) const
   {
      return noconv;
   }
   virtual int do_length( _St& _State , const _To* _F1 , 
      const _To* _L1 , size_t _N2 ) const _THROW0()
   {
      return (_N2 < (size_t)(_L1 - _F1)) ? static_cast<int>(_N2) : static_cast<int>(_L1 - _F1);
   }
   virtual bool do_always_noconv() const _THROW0()
   {
      return true;
   }
   virtual int do_max_length() const _THROW0()
   {
      return 2;
   }
   virtual int do_encoding() const _THROW0()
   {
      return 2;
   }
};


#define IMBUE_NULL_CODECVT( outputFile ) \
{ \
   NullCodecvt* pNullCodecvt = new NullCodecvt; \
   locale loc = locale::classic(); \
   loc._Addfac( pNullCodecvt , NullCodecvt::id, NullCodecvt::_Getcat() ); \
   (outputFile).imbue( loc ); \
}
