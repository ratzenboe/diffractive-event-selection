//=================================
//// include guard
#ifndef __CEPFILTERS_H_INCLUDED__
#define __CEPFILTERS_H_INCLUDED__
//

/* #include <CEPEventBuffer.h> */

Int_t MFilter (CEPEventBuffer*& ev, Int_t cu, Bool_t& isDG, Bool_t& isNDG);
Int_t PFilter (CEPEventBuffer*& ev, Bool_t isMC, Int_t cu, Bool_t& isDG, Bool_t& isNDG);
Int_t LHC16Filter (CEPEventBuffer*& ev, Bool_t isMC, Int_t cu, Bool_t& isDG, Bool_t& isNDG, Int_t mode);
Bool_t checkSPD (CEPEventBuffer *ev, UInt_t tomatch, Bool_t verbose, Int_t mode);

#endif 
