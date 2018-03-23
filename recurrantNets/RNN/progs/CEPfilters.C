#include <list>
#include <iostream>
#include <algorithm>
#include <iterator>

#include <TROOT.h>
#include "TSystem.h" 

/* #include <CEPEventBuffer.h> */
/* #include <CEPfilters.h> */
#define BIT(n) (1ULL << (n))

//-----------------------------------------------------------------------------
Int_t MFilter (CEPEventBuffer*& ev, Int_t cu, Bool_t& isDG, Bool_t& isNDG)
{
  // apply Martin's selection
    
  // initialisation
  Int_t ind;
  UInt_t mask=0, pattern=0;
  TArrayI *masks    = new TArrayI();
  TArrayI *patterns = new TArrayI();
  TArrayI *badinds  = new TArrayI();
  
  // do first some event tests
  isDG = kTRUE;
  isNDG = kTRUE;
  if (cu > 0) {
  
    // pileup events are removed completely
    if (ev->isPileup()) {
      isDG  = kFALSE;
      isNDG = kFALSE;
      return 0;
    }
  }

  Bool_t cleanV0  = kTRUE;
  Bool_t cleanFMD = kTRUE;
  Bool_t cleanAD  = kTRUE;
  
  // !V0
  if (cu > 2) {
    cleanV0 =  cleanV0 && !ev->isV0();
    isDG  = isDG && cleanV0;
    isNDG = isNDG || !cleanV0;
  }
  
  // !FMD
  if (cu > 7) {
    cleanFMD =  !ev->isFMD();
    isDG  = isDG && cleanFMD;       
    isNDG = isNDG || !cleanFMD;
  }
  
  // !AD
  if (cu > 4) {
    cleanAD =  cleanAD && !ev->isAD();
    isDG  = isDG && cleanAD;       
    isNDG = isNDG || !cleanAD;
  }
  
  // 1. no track with !kTTTPCScluster
  // this kills all events from LHC16[k,l], thus in this case skip this ...
  Int_t nTCPSCluster = 0;
  if (ev->GetRunNumber()<256000) {
    mask = AliCEPBase::kTTTPCScluster;
    pattern = 0;
    Int_t nTCPSCluster = ev->GetnTracks(mask,pattern);
  }
    
  // 2. kTTITSpure -> npureITSTracks
  mask = AliCEPBase::kTTITSpure;
  pattern = AliCEPBase::kTTITSpure;
  Int_t npureITSTracks = ev->GetnTracks(mask,pattern);
    
  // 3. kTTDCA && !kTTV0 && !kTTITSpure && kTTZv -> nTrackSel
  mask = AliCEPBase::kTTDCA+AliCEPBase::kTTV0+AliCEPBase::kTTITSpure+
    AliCEPBase::kTTZv;
  pattern =  AliCEPBase::kTTDCA+AliCEPBase::kTTZv;
  Int_t nTrackSel = ev->GetnTracks(mask,pattern);

  // 4. 3. && kTTeta && (kTTAccITSTPC || kTTAccITSSA) -> nTrackAccept
  mask += AliCEPBase::kTTeta;
  pattern += AliCEPBase::kTTeta;
  masks->Set(2);
  patterns->Set(2);
  masks->AddAt(mask+AliCEPBase::kTTAccITSTPC,0);
  masks->AddAt(mask+AliCEPBase::kTTAccITSSA, 1);
  patterns->AddAt(pattern+AliCEPBase::kTTAccITSTPC,0);
  patterns->AddAt(pattern+AliCEPBase::kTTAccITSSA, 1);
  Int_t nTrackAccept = ev->GetnTracks(masks,patterns);
    
  // 5. nTrackSel>=npureITSTracks && nTrackSel>=nTracklets && nTrackAccept==nTrackSel
  Int_t nTracklets = ev->GetnTracklets();
  Bool_t TrackNums =
    (nTrackSel>=npureITSTracks && nTrackSel>=nTracklets && nTrackAccept==nTrackSel);
  
  // 6. pass fpassedFiredChipsTest
  
  //printf("\nMartins selection\n");
  //printf("nTCPSCluster: %i\n",nTCPSCluster);
  //printf("npureITSTracks: %i\n",npureITSTracks);
  //printf("nTrackSel: %i\n",nTrackSel);
  //printf("nTrackAccept: %i\n",nTrackAccept);
  //printf("nTracklets: %i\n",nTracklets);
  
  // combine Martin's selection criteria
  Bool_t eventstat = (nTCPSCluster==0) && TrackNums;
    
  // remove the bad tracks from ev
  if (eventstat) {
    masks->Set(4);
    patterns->Set(4);
    masks->AddAt(AliCEPBase::kTTDCA,0);
    masks->AddAt(AliCEPBase::kTTV0, 1);
    masks->AddAt(AliCEPBase::kTTITSpure, 2);
    masks->AddAt(AliCEPBase::kTTZv, 3);
    patterns->AddAt(0,0);
    patterns->AddAt(AliCEPBase::kTTV0,1);
    patterns->AddAt(AliCEPBase::kTTITSpure,2);
    patterns->AddAt(0,3);
    Int_t nbadtracks = ev->GetnTracks(masks,patterns,badinds);
  
    // remove bad tracks from event
    for (Int_t jj=0; jj<nbadtracks; jj++) {
      ind = badinds->At(jj)-jj;
      ev->RemoveTrack(ind);
    }
    
  } else {
    nTrackAccept = 0;
    
    isDG  = kFALSE;
    isNDG = kFALSE;
  }
  
  // clean up
  delete masks;
  delete patterns;
  delete badinds;
  
  return nTrackAccept;
  
}

//-----------------------------------------------------------------------------
Int_t PFilter (CEPEventBuffer*& ev, Bool_t isMC, Int_t cu,
  Bool_t& isDG, Bool_t& isNDG)
{
  // the cut values is composed of bits
  //  0 =     1: removes pileups                           
  //  1 =     2: CCUP13 fired or was successfully replaied 
  //  2 =     4: CCUP25 fired or was successfully replaied 
  //  3 =     8: !V0                                       
  //  4 =    16: !FMD                                      
  //  5 =    32: !AD                                       
  //  6 =    64: no tracks with TPCclusters > 3            
  //  7 =   128: select only "good" tracks                 
  //  8 =   256: remove events with "bad" selected tracks  
  //  9 =   512: select V0-daughters                       
  // 10 =  1024: exclude V0-daughters                      
  // 11 =  2048: require CINT11                    
  // 12 =  4096: require CCUP2                     
  // 13 =  8192: require CCUP13                    
  // 14 = 16384: require CCUP25                     
  //
  // ---------------------------------------------------------------------
  // 0 1 2 3  4  5  6   7   8   9   10   11   12   13    14                 
  // 1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384                         
  // ---------------------------------------------------------------------
  // 1 1 0 0  0  0  0   0   0   0    0    0    0    0     0  =     3      
  // 1 0 1 0  0  0  0   0   0   0    0    0    0    0     0  =     5      
  // 1 1 0 1  0  0  0   0   0   0    0    0    0    0     0  =    11      
  // 1 1 1 1  0  0  0   0   0   0    0    0    0    0     0  =    15      
  // 1 1 0 1  1  0  0   0   0   0    0    0    0    0     0  =    27      
  // 1 1 0 1  0  1  0   0   0   0    0    0    0    0     0  =    43      
  // 1 1 0 1  1  1  0   0   0   0    0    0    0    0     0  =    59      
  // 1 1 0 1  0  0  0   1   0   0    0    0    0    0     0  =   139      
  // 1 1 0 1  0  0  0   0   1   0    0    0    0    0     0  =   267      
  // 1 1 0 1  0  1  0   0   1   0    0    0    0    0     0  =   299      
  // 1 1 0 1  0  0  0   1   1   0    0    0    0    0     0  =   395      
  // 1 1 0 0  0  0  0   0   0   1    0    0    0    0     0  =   515      
  // 1 1 0 0  0  0  0   0   0   0    1    0    0    0     0  =  1027      
  // 1 1 0 0  0  0  0   0   0   0    0    1    0    0     0  =  2051      
  // 1 0 1 0  0  0  0   0   0   0    0    1    0    0     0  =  2053      
  // 1 1 0 1  0  0  0   1   0   0    0    1    0    0     0  =  2187      
  // 1 1 0 1  0  1  0   1   1   0    1    1    0    0     0  =  3499     
  // 1 1 0 0  0  0  0   0   0   0    0    0    1    0     0  =  4099      
  // 1 1 0 0  0  0  0   0   0   0    0    0    0    1     0  =  8195      
  // 1 0 1 0  0  0  0   0   0   0    0    0    0    1     0  =  8197       
  // 1 1 0 1  0  0  0   1   0   0    0    0    0    1     0  =  8331      
  // 1 1 0 1  0  1  0   1   1   0    1    0    0    1     0  =  9643    
  // 1 1 0 0  0  0  0   0   0   0    0    0    0    0     1  = 16387       
  // ---------------------------------------------------------------------
    
  // initialisation
  TString firedTriggerClasses;
  Bool_t DGtested = kFALSE;
  UInt_t mask=0, pattern=0;
  TArrayI *masks    = new TArrayI();
  TArrayI *patterns = new TArrayI();
  Int_t nbadtracks;
  Int_t ind;
  TArrayI *goodinds = new TArrayI();
  TArrayI *badinds  = new TArrayI();
  
  Bool_t isCINT11     = kFALSE;
  Bool_t isCCUP2_SPD1 = kFALSE;
  Bool_t isCCUP13     = kFALSE;
  Bool_t isCCUP25     = kFALSE;

  Bool_t *fPFBBFlagV0;
  Bool_t *fPFBGFlagV0;
  Bool_t *fPFBBFlagAD;
  Bool_t *fPFBGFlagAD;

  // alert if collission type is !0
  if (ev->GetCollissionType() != 0) printf("Event type: %i\n",ev->GetCollissionType());

  // start with some trivial tests
  isDG = kTRUE;
  isNDG = kTRUE;
  if (cu & BIT(0)) {
    // pileup events are removed completely
    if (ev->isPileup()) {
      isDG  = kFALSE;
      isNDG = kFALSE;
      return 0;
    }
  }

  // has DGtrigger fired or was successfully replaied
  Int_t bitword = BIT(1) | BIT(2) | BIT(11) | BIT(12) | BIT(13) | BIT(14);
  if ( cu & bitword ) {

    // check the fired trigger classes
    firedTriggerClasses = ev->GetFiredTriggerClasses();
    isCINT11     = firedTriggerClasses.Contains("CINT11-B-NOPF-CENTNOTRD");
    isCCUP2_SPD1 = firedTriggerClasses.Contains("CCUP2-B-SPD1-CENTNOTRD");
    isCCUP13     = firedTriggerClasses.Contains("CCUP13-B-SPD1-CENTNOTRD");
    isCCUP25     = firedTriggerClasses.Contains("CCUP25-B-SPD1-CENTNOTRD");
   
    // there are several possibilities for isDGTrigger to be true
    // 1. CCUP13 has fired
    // 2. CCUP25 has fired
    // 3. CCUP2 or CINT11 has fired and DGtrigger was successfully replaied
    isDG  = isDG  && ev->isDGTrigger();   // CCUP13 || CCUP25
    isNDG = isNDG && !ev->isDGTrigger();
    
    
    // require in addition a minimum of 2 hits on TOFmaxipads
    if (cu & BIT(2)) {
      isDG  = isDG  && (ev->GetnTOFmaxipads()>=2);   // CCUP25 tigger
      isNDG = isNDG && (ev->GetnTOFmaxipads()< 2);
    }
    // requite specific triggers to have fired
    if (cu & BIT(11) && !isCINT11)     return 0;   // not CINT11 tigger
    if (cu & BIT(12) && !isCCUP2_SPD1) return 0;   // not CCUP2 tigger
    if (cu & BIT(13) && !isCCUP13)     return 0;   // not CCUP13 tigger
    if (cu & BIT(14) && !isCCUP25)     return 0;   // not CCUP25 tigger
      
    DGtested = kTRUE;
    
    /*
    // printout the type of trigger
    TString fT = ev->GetFiredTriggerClasses();
    
    printf("fired triggers %i:",isDG);
    if (fT.Contains("CCUP2-B-SPD1-CENTNOTRD"))
      printf(" CCUP2 (%i, %i)",ev->GetisSTGTriggerFired(),ev->GetnTOFmaxipads());
    if (fT.Contains("CINT11-B-NOPF-CENTNOTRD"))
      printf(" CINT11 (%i, %i)",ev->GetisSTGTriggerFired(),ev->GetnTOFmaxipads());
    if (fT.Contains("CCUP13-B-SPD1-CENTNOTRD"))
      printf(" CCUP13 (%i, %i)",ev->GetisSTGTriggerFired(),ev->GetnTOFmaxipads());
    if (fT.Contains("CCUP25-B-SPD1-CENTNOTRD"))
      printf(" CCUP25 (%i, %i)",ev->GetisSTGTriggerFired(),ev->GetnTOFmaxipads());
    printf("\n");
    */
  }

  // test of V0, AD, and FMD
  Bool_t cleanV0  = kTRUE;
  Bool_t cleanFMD = kTRUE;
  Bool_t cleanAD  = kTRUE;
  
  // !V0
  if (cu & BIT(3)) {
    if (!isMC) {
      fPFBBFlagV0 = ev->GetPFBBFlagV0();
      fPFBGFlagV0 = ev->GetPFBGFlagV0();
      for (Int_t ii=2; ii<17; ii++)        // +- 7 bc
        cleanV0 =  cleanV0 && !fPFBBFlagV0[ii] && !fPFBGFlagV0[ii];
    }
    cleanV0 =  cleanV0 && !ev->isV0();
    isDG  = isDG && cleanV0;
    isNDG = isNDG || !cleanV0;
    DGtested = kTRUE;
  }
  
  // !FMD
  if (cu & BIT(4)) {
    cleanFMD =  !ev->isFMD();
    isDG  = isDG && cleanFMD;       
    isNDG = isNDG || !cleanFMD;
    DGtested = kTRUE;
  }
  
  // !AD
  if (cu & BIT(5)) {
    if (!isMC) {
      fPFBBFlagAD = ev->GetPFBBFlagAD();
      fPFBGFlagAD = ev->GetPFBGFlagAD();
      for (Int_t ii=3; ii<17; ii++)         // +- 7 bc
        cleanAD =  cleanAD && !fPFBBFlagAD[ii] && !fPFBGFlagAD[ii];
    }
    // if (!ev->isAD()) printf("AD: %i %i\n",cleanAD,!ev->isAD());
    cleanAD =  cleanAD && !ev->isAD();
    isDG  = isDG && cleanAD;       
    isNDG = isNDG || !cleanAD;
    DGtested = kTRUE;
  }

  // no tracks with TPCclusters > 3
  Int_t ntrkTPCScluster;
  if (cu & BIT(6)) {
    mask    = 1*AliCEPBase::kTTTPCScluster;
    pattern = 1*AliCEPBase::kTTTPCScluster;
    ntrkTPCScluster = ev->GetnTracks(mask,pattern);
    if (ntrkTPCScluster > 0) {
      isDG  = kFALSE;
      isNDG = kFALSE;
      return 0;
    }
  }

  // check the tracks
  Bool_t goodTracks    = kTRUE;
  Int_t nTracks        = ev->GetnTracks();
  Int_t nTrackAccept   = nTracks;
  Int_t nTrackSel      = nTracks;
  Int_t npureITSTracks = 0;
  Int_t nTracklets     = 0;
  
  // initialize goodinds
  goodinds->Set(nTracks);
  for (Int_t ii=0; ii<nTracks; ii++) goodinds->SetAt(ii,ii);
    
  // 1. good track selection
  if (cu & BIT(7)) {
    mask    =
      1*AliCEPBase::kTTAccITSTPC +
      1*AliCEPBase::kTTDCA +
      // 1*AliCEPBase::kTTV0 +
      1*AliCEPBase::kTTITSpure +
      1*AliCEPBase::kTTZv;
    pattern =
      1*AliCEPBase::kTTAccITSTPC +
      1*AliCEPBase::kTTDCA +
      // 0*AliCEPBase::kTTV0 +
      0*AliCEPBase::kTTITSpure +
      1*AliCEPBase::kTTZv;
    nTrackSel = ev->GetnTracks(mask,pattern,goodinds);
  }

  // 2. additional required track criteria
  nTrackAccept = nTrackSel;
  if (cu & BIT(8)) {
    // number of pure ITS tracks and tracklets
    mask    = 1*AliCEPBase::kTTITSpure;
    pattern = 1*AliCEPBase::kTTITSpure;
    npureITSTracks = ev->GetnTracks(mask,pattern);
  
    nTracklets     = ev->GetnTracklets();

    mask    =
      1*AliCEPBase::kTTDCA +
      1*AliCEPBase::kTTV0 +
      1*AliCEPBase::kTTITSpure +
      1*AliCEPBase::kTTZv+
      1*AliCEPBase::kTTeta+
      1*AliCEPBase::kTTFiredChips;
    pattern =
      1*AliCEPBase::kTTDCA +
      0*AliCEPBase::kTTV0 +
      0*AliCEPBase::kTTITSpure +
      1*AliCEPBase::kTTZv+
      1*AliCEPBase::kTTeta+
      1*AliCEPBase::kTTFiredChips;
    
    masks->Set(2);
    patterns->Set(2);
    masks->AddAt(mask+1*AliCEPBase::kTTAccITSTPC,0);
    masks->AddAt(mask+1*AliCEPBase::kTTAccITSSA, 1);
    patterns->AddAt(pattern+1*AliCEPBase::kTTAccITSTPC,0);
    patterns->AddAt(pattern+1*AliCEPBase::kTTAccITSSA, 1);
    
    nTrackAccept = ev->GetnTracks(masks,patterns,goodinds);
    
  }
  
  // for test purposes select V0-daughters
  if (cu & BIT(9)) {
    mask    = 1*AliCEPBase::kTTV0;
    pattern = 1*AliCEPBase::kTTV0;
  
    nTrackSel = ev->GetnTracks(mask,pattern,goodinds);
    nTrackAccept = nTrackSel;
    // printf("Number of V0-daughters %i\n",nTrackSel);
    
  }

  // for test purposes exclude V0-daughters
  if (cu & BIT(10)) {
    mask    = 1*AliCEPBase::kTTV0;
    pattern = 0*AliCEPBase::kTTV0;
  
    nTrackSel = ev->GetnTracks(mask,pattern,goodinds);
    nTrackAccept = nTrackSel;
    
  }

  // are there any tracks selected
  goodTracks =
    nTrackSel >= npureITSTracks &&
    nTrackSel>nTracklets &&
    nTrackAccept==nTrackSel;  
  
  // remove the bad tracks from ev
  if (goodTracks) {
    
    // remove badtracks if any from the event buffer
    Bool_t gg;
    if (nTrackAccept != nTracks) {
      nbadtracks = nTracks-nTrackAccept;
      badinds->Set(nbadtracks);
      // printf("Number of tracks to be removed: %i of %i\n",nbadtracks,nTracks);
          
      Int_t cc = 0;
      for (Int_t ii=0; ii<nTracks; ii++) {
        gg = kFALSE;
        for (Int_t jj=0; jj<nTrackAccept; jj++)
          if (goodinds->At(jj) == ii) gg = kTRUE;
        if (!gg) {
          badinds->SetAt(ii,cc);
          cc++;
        }
      }
    
      // remove bad tracks from event
      cc = 0;
      for (Int_t ii=0; ii<nbadtracks; ii++) {
        ind = badinds->At(ii)-cc;
        ev->RemoveTrack(ind);
        cc++;
      }
    }
    
  } else {
    nTrackAccept = 0;
  }
  // if no DG test was performed then isDG=kFALSE and isNDG=kTRUE
  isDG = isDG && DGtested;
  
  // printf("Final number of tracks: %i\n",ev->GetnTracks());

  // clean up
  delete masks;
  delete patterns;
  delete goodinds;
  delete badinds;
  
  return nTrackAccept;
  
}

//-----------------------------------------------------------------------------
  // --------------------------------------------------------------------
  // 0 1 2 3  4  5  6   7   8   11   12   13    14    15    16          
  // 1 2 4 8 16 32 64 128 256 2048 4096 8192 16384 32768 65536         
  // ---------------------------------------------------------------------
  // 0 1 0 0  0  0  0   1   0    0    0    0     0     0     0  =   (   66)
  // 0 1 0 1  0  1  0   1   0    0    0    0     0     0     0  =   (  170)
  //
  // 0 0 0 0  0  0  0   0   0    0    1    0     0     0     0  =   ( 2048)
  // 0 1 0 0  0  0  0   0   0    0    1    0     0     0     0  =   ( 2050)
  // 0 1 0 0  0  0  0   1   0    0    1    0     0     0     0  =   ( 2114)
  // 0 1 0 1  0  1  0   1   0    0    1    0     0     0     0  =   ( 2154)
  // 0 1 0 0  0  0  0   1   1    0    1    0     0     0     0  =   ( 2242)
  // 0 1 0 0  0  0  0   1   1    1    1    0     0     0     0  =   ( 2498)
  // 0 1 0 1  0  0  0   1   1    1    1    0     0     0     0  =   ( 2506)
  // 0 1 0 1  0  1  0   1   1    1    1    0     0     0     0  =   ( 2538)
  // 0 1 0 1  1  1  0   1   1    1    1    0     0     0     0  =   ( 2554)
  //
  // 0 1 0 1  0  1  0   1   0    0    0    1     0     0     0  =   ( 4202)
  //
  // 0 1 0 0  0  0  0   0   0    0    0    0     1     0     0  =   ( 8194)
  // 0 1 0 0  0  0  0   1   0    0    0    0     1     0     0  =   ( 8258)
  // 0 1 0 1  0  0  0   1   0    0    0    0     1     0     0  =   ( 8266)
  // 0 1 0 1  0  1  0   1   0    0    0    0     1     0     0  =   ( 8298)
  // 0 1 0 0  0  0  0   1   1    0    0    0     1     0     0  =   ( 8386)
  // 0 1 0 0  0  0  0   1   1    1    0    0     1     0     0  =   ( 8642)
  // 0 1 0 1  0  0  0   1   1    1    0    0     1     0     0  =   ( 8650)
  // 0 1 0 1  0  1  0   1   1    1    0    0     1     0     0  =   ( 8682)
  // 0 1 0 1  1  1  0   1   1    1    0    0     1     0     0  =   ( 8698)
  //
  // 0 0 1 1  0  1  0   1   0    0    0    0     0     0     0  =   (  108)
  // 0 0 1 1  0  1  0   1   0    0    1    0     0     0     0  =   ( 2156)
  // 0 1 1 1  0  1  0   1   0    0    1    0     0     0     0  =   ( 2158)
  // 0 1 0 1  0  1  0   1   0    0    0    0     1     0     0  =   ( 8298)
  // 0 0 1 1  0  1  0   1   0    0    0    0     1     0     0  =   ( 8300)
  // 0 0 1 1  0  1  0   1   0    0    0    0     0     1     0  =   (16492)
  // 0 0 1 1  0  1  0   1   0    1    0    0     0     1     0  =   (16748)
  //
  // 0 1 0 1  0  1  0   1   0    0    0    0     0     0     1  =   (32874)
  //
  // ---------------------------------------------------------------------
  // the cut values is composed of bits
  //  0 =     1: removes pileups                           
  //  1 =     2: CCUP13 successfully replaied 
  //  2 =     4: CCUP25 successfully replaied 
  //  3 =     8: !V0                                       
  //  4 =    16: !FMD                                      
  //  5 =    32: !AD                                       
  //  6 =    64: *FO>=1 (to replay OSMB) && *FO<=ntrk
  //  7 =   128: number of online tracklets >= 2 (to replay OSTG(0))
  //  8 =   256: no ITSPure
  //  9 =   512: TPCOnly tracks, SPD hit, no ITSPure
  // 10 =  1024: V0 tracks
  // 11 =  2048: require CINT11 fired
  // 12 =  4096: require CCUP2-B-SPD1-CENTNOTRD fired                     
  // 13 =  8192: require CCUP13 fired                    
  // 14 = 16384: require CCUP25 fired                     
  // 15 = 32768: require CCUP2-B-NOPF-CENTNOTRD fired                     
  //
  // new-new-new-new-new-new-new-new-new-new-new-new-new-new-new-new-new
  // -------------------------------------------------------------------
  // 0 1 2 3  4  5  6   7   8   9   10   11   12   13    14    15      
  // 1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384 32768     
  // -------------------------------------------------------------------
  //
  // Simulations
  // 0 0 0 1  0  0  0   1   0   0    0    0    0    0     0     0    136
  // 0 1 0 1  0  0  0   1   0   0    0    0    0    0     0     0    138
  // 1 1 0 0  0  0  1   0   0   0    0    0    0    0     0     0     67
  // 1 1 0 1  0  0  1   0   0   0    0    0    0    0     0     0     74
  // 1 1 0 0  0  1  1   0   0   0    0    0    0    0     0     0     99
  // 1 1 0 1  0  0  1   1   0   0    0    0    0    0     0     0    203
  // 1 1 0 1  0  1  0   1   0   0    0    0    0    0     0     0    171
  // 1 1 0 1  0  1  1   1   0   0    0    0    0    0     0     0    235
  // 1 1 0 1  0  1  1   1   0   1    0    0    0    0     0     0    747
  // !AD & !V0
  // 1 1 0 1  0  1  1   0   0   0    0    0    0    0     0     0    107
  //
  // CINT11
  // 0 0 0 0  0  0  0   0   0   0    0    1    0    0     0     0   2048
  // 0 1 0 0  0  0  1   0   0   0    0    1    0    0     0     0   2114
  // 1 0 0 0  0  0  0   0   0   0    0    1    0    0     0     0   2049
  // 1 1 0 0  0  0  0   0   0   0    0    1    0    0     0     0   2051
  // 1 1 0 0  0  0  1   0   0   0    0    1    0    0     0     0   2115
  // 1 1 0 0  0  1  1   0   0   0    0    1    0    0     0     0   2147
  // 1 1 0 1  0  0  0   0   0   0    0    1    0    0     0     0   2059
  // 1 1 0 1  0  0  0   1   0   0    0    1    0    0     0     0   2187
  // 1 1 0 1  0  0  1   1   0   0    0    1    0    0     0     0   2251
  // 1 1 0 1  0  0  1   1   1   0    0    1    0    0     0     0   2507
  // 1 1 0 1  0  1  1   1   1   0    0    1    0    0     0     0   2539
  
  // 1 1 0 1  0  0  1   1   0   0    0    1    0    0     0     0   2251
  // 1 1 0 1  0  1  1   1   0   0    0    1    0    0     0     0   2283
  // 1 1 0 1  0  0  1   1   0   1    0    1    0    0     0     0   2763
  // 1 1 0 1  0  1  1   1   0   1    0    1    0    0     0     0   2795
  //
  // CCUP13
  // 0 0 0 0  0  0  0   0   0   0    0    0    0    1     0     0   8192
  // 1 0 0 0  0  0  0   0   0   0    0    0    0    1     0     0   8193
  // 1 1 0 0  0  0  0   0   0   0    0    0    0    1     0     0   8195
  // 0 1 0 0  0  0  1   0   0   0    0    0    0    1     0     0   8258
  // 1 1 0 0  0  0  1   0   0   0    0    0    0    1     0     0   8259
  // 1 1 0 0  0  1  1   0   0   0    0    0    0    1     0     0   8291
  // 1 1 0 0  0  0  1   0   1   0    0    0    0    1     0     0   8515
  // 1 1 0 0  0  0  1   0   1   1    0    0    0    1     0     0   9027
  // 1 1 0 0  0  1  1   0   1   1    0    0    0    1     0     0   9059

  // 1 1 0 1  0  0  1   1   0   0    0    0    0    1     0     0   8395
  // 1 1 0 1  0  1  1   1   0   0    0    0    0    1     0     0   8427
  // 1 1 0 1  0  0  1   1   0   1    0    0    0    1     0     0   8907
  // 1 1 0 1  0  1  1   1   0   1    0    0    0    1     0     0   8939
  //
  // 1 1 0 1  0  1  1   0   0   0    0    0    0    1     0     0   8299
  // 1 1 0 1  0  1  0   1   0   0    0    0    0    1     0     0   8363
  // 1 1 0 1  0  1  0   1   1   0    0    0    0    1     0     0   8619
  // 1 1 0 1  0  1  1   1   1   0    0    0    0    1     0     0   8683
  // 1 1 0 1  1  1  1   1   1   0    0    0    0    1     0     0   8699
  // 
  // -------------------------------------------------------------------
  // 16 = 65536: additional cuts to clean the event
  //
  // as basis for the CCUP13 data use cut 8259
  //
  // 0 1 2 3  4  5  6   7   8   9   10   11   12   13    14    15    16    
  // 1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384 32768 65536   
  // 1 1 0 0  0  0  0   0   0   0    0    0    0    0     0     0     1  65539
  // 0 1 0 0  0  0  0   0   0   0    0    1    0    0     0     0     1  67586
  // 0 1 0 0  0  1  0   0   0   0    0    1    0    0     0     0     1  67618
  //
  // 0 1 0 0  0  0  0   0   0   0    0    0    0    1     0     0     1  73730
  // 0 1 0 1  0  0  0   0   0   0    0    0    0    1     0     0     1  73738
  // 0 1 0 1  0  1  0   0   0   0    0    0    0    1     0     0     1  73770
  // 1 1 0 0  0  0  1   0   0   0    0    0    0    1     0     0     1  73795
                   
Int_t LHC16Filter (CEPEventBuffer*& ev, Bool_t isMC, Int_t cu, Bool_t& isDG, Bool_t& isNDG, Int_t mode)
{
  // initialisation
  TString firedTriggerClasses;
  Bool_t DGtested = kFALSE;
  UInt_t mask=0, pattern=0;
  Int_t ind;
  TArrayI *goodinds = new TArrayI();
  TArrayI *badinds  = new TArrayI();
  
  Bool_t isCINT11     = kFALSE;
  Bool_t isCCUP2_SPD1 = kFALSE;
  Bool_t isCCUP2_NOPF = kFALSE;
  Bool_t isCCUP13     = kFALSE;
  Bool_t isCCUP25     = kFALSE;

  Bool_t *fPFBBFlagV0;
  Bool_t *fPFBGFlagV0;
  Bool_t *fPFBBFlagAD;
  Bool_t *fPFBGFlagAD;

  // alert if collission type is !0
  if (ev->GetCollissionType() != 0) printf("Event type: %i\n",ev->GetCollissionType());


  // check if V0, FMD, AD have been activated
  // apply past-future protection for V0 and AD decisions
  // V0
  Bool_t cleanV0  = kTRUE;
  if (!isMC) {
    fPFBBFlagV0 = ev->GetPFBBFlagV0();
    fPFBGFlagV0 = ev->GetPFBGFlagV0();
    for (Int_t ii=3; ii<18; ii++)        // -+ 7 bc
      cleanV0 =  cleanV0 && !fPFBBFlagV0[ii] && !fPFBGFlagV0[ii];
  }
  cleanV0 =  cleanV0 && !ev->isV0();

  // FMD
  Bool_t cleanFMD = kTRUE;
  cleanFMD =  !ev->isFMD();

  // AD
  Bool_t cleanAD  = kTRUE;
  if (!isMC) {
    fPFBBFlagAD = ev->GetPFBBFlagAD();
    fPFBGFlagAD = ev->GetPFBGFlagAD();
    for (Int_t ii=3; ii<18; ii++)         // -+ 7 bc
      cleanAD =  cleanAD && !fPFBBFlagAD[ii] && !fPFBGFlagAD[ii];
  }
  cleanAD =  cleanAD && !ev->isAD();

  // start with some trivial tests
  isDG = kFALSE;
  isNDG = kFALSE;
  if (cu & BIT(0)) {
    // pileup events are removed completely
    if (ev->isPileup()) return 0;
  }

  // check the fired trigger classes
  firedTriggerClasses = ev->GetFiredTriggerClasses();
  // printf("Fired trigger classes:\n%s\n",firedTriggerClasses.Data());
  isCINT11     = firedTriggerClasses.Contains("CINT11-B-NOPF-CENTNOTRD");
  isCCUP2_SPD1 = firedTriggerClasses.Contains("CCUP2-B-SPD1-CENTNOTRD");
  isCCUP2_NOPF = firedTriggerClasses.Contains("CCUP2-B-NOPF-CENTNOTRD");
  isCCUP13     = firedTriggerClasses.Contains("CCUP13-B-SPD1-CENTNOTRD");
  isCCUP25     = firedTriggerClasses.Contains("CCUP25-B-SPD1-CENTNOTRD");

  // require specific triggers to have fired
  if (cu & BIT(11) && !isCINT11)     return 0;   // not CINT11 tigger
  if (cu & BIT(12) && !isCCUP2_SPD1) return 0;   // not CCUP2-SPD tigger
  if (cu & BIT(13) && !isCCUP13)     return 0;   // not CCUP13 tigger
  if (cu & BIT(14) && !isCCUP25)     return 0;   // not CCUP25 tigger
  if (cu & BIT(15) && !isCCUP2_NOPF) return 0;   // not CCUP2-NOPF tigger

  // reset isDG and isNDG
  isDG = kTRUE;
  isNDG = kTRUE;
  if ( BIT(13) || BIT(14) ) DGtested = kTRUE;
  
  // replay CCUP13
  // CCUP13 = !OVBA & !OVBC & OSTG(2016)
  if ( cu & BIT(1) ) {
    isDG  = isDG && cleanV0 && ev->GetisSTGTriggerFired();
    DGtested = kTRUE;
  }
    
  // replay CCUP25
  // CCUP13 = !OVBA & !OVBC & OSTG(2017) & OOM2
  if ( cu & BIT(2) ) {
    isDG  = isDG && cleanV0 && (ev->GetisSTGTriggerFired() & (1<<4)) && (ev->GetnTOFmaxipads()>=2);
    DGtested = kTRUE;
  }

  // !V0
  if (cu & BIT(3)) {
    isDG  = isDG && cleanV0;
    isNDG = isNDG || !cleanV0;
    DGtested = kTRUE;
  }
  
  // !FMD
  if (cu & BIT(4)) {
    isDG  = isDG && cleanFMD;       
    isNDG = isNDG || !cleanFMD;
    DGtested = kTRUE;
  }
  
  // !AD
  if (cu & BIT(5)) {
    isDG  = isDG && cleanAD;       
    isNDG = isNDG || !cleanAD;
    DGtested = kTRUE;
  }
  
  // check number of online tracklets
  // OSTG(2016)
  if (cu & BIT(7)) {
    isDG = isDG && ev->GetisSTGTriggerFired();
    DGtested = kTRUE;
  }

  // check the tracks
  Int_t nTracks       = ev->GetnTracks();
  Int_t nTrackAccept  = nTracks;
  
  // no ITSPure tracks
  if (cu & BIT(8)) {
    mask    =
      1*AliCEPBase::kTTITSpure;
    pattern =
      1*AliCEPBase::kTTITSpure;
    isDG  = isDG  && (ev->GetnTracks(mask,pattern) == 0);
    isNDG = isNDG && (ev->GetnTracks(mask,pattern) == 0);
    DGtested = kTRUE;
  }

  // initialize goodinds
  goodinds->Set(nTracks);
  for (Int_t ii=0; ii<nTracks; ii++) goodinds->SetAt(ii,ii);
    
  // good track selection
  UInt_t bitword = BIT(9) | BIT(10);
  if ( cu & bitword ) {
    if (cu & BIT(9)) {
      mask    =
        1*AliCEPBase::kTTAccTPCOnly +
        1*AliCEPBase::kTTSPDHit +
        1*AliCEPBase::kTTITSpure;
      pattern =
        1*AliCEPBase::kTTAccTPCOnly +
        1*AliCEPBase::kTTSPDHit +
        0*AliCEPBase::kTTITSpure;
      nTrackAccept  = ev->GetnTracks(mask,pattern,goodinds);
    } else {
      mask    = 1*AliCEPBase::kTTV0;
      pattern = 1*AliCEPBase::kTTV0;
      nTrackAccept  = ev->GetnTracks(mask,pattern,goodinds);
    }
        
    // remove badtracks if any from the event buffer
    Bool_t gg;
    if (nTrackAccept != nTracks) {
      Int_t nbadtracks = nTracks-nTrackAccept;
      badinds->Set(nbadtracks);
      // printf("Number of tracks to be removed: %i of %i\n",nbadtracks,nTracks);
          
      Int_t cc = 0;
      for (Int_t ii=0; ii<nTracks; ii++) {
        gg = kFALSE;
        for (Int_t jj=0; jj<nTrackAccept; jj++)
          if (goodinds->At(jj) == ii) gg = kTRUE;
        if (!gg) {
          badinds->SetAt(ii,cc);
          cc++;
        }
      }
    
      // remove bad tracks from event
      cc = 0;
      for (Int_t ii=0; ii<nbadtracks; ii++) {
        ind = badinds->At(ii)-cc;
        ev->RemoveTrack(ind);
        cc++;
      }
    }
    
  }
  
  // check number of fired chips to replay OSMB(2016/2017)
  // use online and offline decisions
  if (cu & BIT(6)) {
    Bool_t firedChipsOK = kTRUE;
    for (Int_t ii=0; ii<4; ii++)
      firedChipsOK =
        firedChipsOK &&
        (ev->GetnFiredChips(ii)>=1) &&
        (ev->GetnFiredChips(ii)<=nTrackAccept);
    isDG = isDG && firedChipsOK;
    DGtested = kTRUE;
  }

  // extra cuts to clean event
  if (cu & BIT(16)) {
    
    //
    UInt_t tomatch;
    tomatch = 1+2+4+8+16+32+64;
    if (!checkSPD(ev,tomatch,kFALSE,mode)) { 
      //checkSPD(ev,tomatch,kTRUE);
      return 0;
    }
    
  }
  
  // if no DG test was performed then isDG=kFALSE and isNDG=kTRUE
  isDG = isDG && DGtested;
  if (isDG) isNDG = kFALSE;
  
  // printf("Final number of tracks: %i\n",ev->GetnTracks());

  // clean up
  delete goodinds;
  
  return nTrackAccept;
  
}

// ----------------------------------------------------------------------------
// proper event selection with SPD
//
// 1. all SPD clusters which belong to a tracklet have a properly associated
//    track and ...
//    ... the associated track is the same for the cluster on layer 1
//    and layer 2
//
// 2. all associated tracks pass TPCOnly track cuts and ...
//    have hits on both SPD layers as well as more TPC clusters than minnTPCclu
//    Additional track cuts could be added here, e.g.
//      . track belongs to main vertex
//      . small dca
//      . 
//
// 3. the number of singles is smaller than or equal to maxnSingle
//
// 4. the number of clusters on SPD layer 1 and layer2 is <=
//    (the number of tracklets+maxdnclu)
//
// 5. the number of SPD fired chips of layer 1 and layer 2 is <=
//    (the number of tracklets+maxdnfchips)
//
// 6. the number of SPD fired FastOR chips of layer 1 and layer 2 is <=
//    (the number of tracklets+maxdnfFOchips)
// 
//
//  the number of clusters on SPD layer 1 and layer 2 is obtained with
//    nclu1 = mult->GetNumberOfITSClusters(0)
//    nclu2 = mult->GetNumberOfITSClusters(1)
//
//  the number of SPD fired chips of layer 1 and layer 2 is obtained with
//    nfchip1 = mult->GetNumberOfFiredChips(0)
//    nfchip2 = mult->GetNumberOfFiredChips(1)
//
//  the number of SPD fired FastOR chips of layer 1 and layer 2 is obtained with
//    TBits foMap = mult->GetFastOrFiredChips()
//    for (Int_t ii=0; ii<400; ii++)    nffochips1 += foMap[ii]>0 ? 1 : 0;
//    for (Int_t ii=400; ii<1200; ii++) nffochips2 += foMap[ii]>0 ? 1 : 0;
//
// in the ideal case ...
//    maxnSingle = maxdnclu = maxdnfchips = maxdnfFOchips = 0
// larger values will lead to increased level of contamination (to be tested)
//
Bool_t checkSPD (CEPEventBuffer *ev, UInt_t tomatch, Bool_t verbose, Int_t mode=0)
{
  
  // define some thresholds
  Int_t minnTPCclus   = 0;
  Int_t maxnSingle    = 0;
  Int_t maxdnclu      = 10000;
  Int_t maxdnfchips   = 10000;
  Int_t maxdnfFOchips = 10000;
  
  if (mode==0) { maxdnclu=0; maxdnfchips=1; maxdnfFOchips=1; }
  if (mode==2) { maxnSingle=1; }
  if (mode==3) { maxnSingle=2; }
  if (mode==4) { maxnSingle=10000; }

  // variables
  std::list<Int_t> assoctrklist;
  std::list<Int_t> tpconlylist;
  UChar_t ITSncls;
  Int_t trind;
  UInt_t trkstat;
  TObjArray* trl2tr;
  TVector2 *vec = new TVector2(), *vectmp;

  // to check
  Bool_t goodevent;
  Bool_t trksassocok;   // both clusters belonging to a tracklet are associated with the same track
  Bool_t assoctrksok;   // all associated tracks pass TPCOnly and have hits on SPD Ll/L2 and TPC
  Bool_t singleok;      // number of singles <= maxnSingle;
  Bool_t SPDhitok;      // number of SPD-layer-1 and -2 clusters <= number of tracklets+maxdnclu
  Bool_t SPDfchipsok;   // number of SPD fired chips of layer 1 and layer 2 <= number of tracklets+maxdnfchips
  Bool_t SPDfFOchipsok; // number of SPD fired FastOR chips of layer 1 and layer 2 <= number of tracklets+maxdnfchips
  Bool_t vtxok;         // vertex resolution and position
  
  // 1. check if all the clusters which are part of a tracklet are also
  // associated with a track, both clusters of a tracklet must be associated
  // with the same track make a list of all associated tracks -> assoctrklist
  trksassocok = kTRUE;
  
  assoctrklist.clear();
  Int_t ntrl = ev->GetnTracklets();
  trl2tr = ev->GetTrl2Tr();
  
  for (Int_t jj=0; jj<ntrl; jj++) {
    vec->Set(-1.,-1.);
    if (jj<trl2tr->GetEntries()) {
      vectmp = (TVector2*) trl2tr->At(jj);
      vec->Set(vectmp->X(),vectmp->Y());
    }
    
    if (vec->X()>=0) assoctrklist.push_front((Int_t)vec->X());
    if (vec->Y()>=0) assoctrklist.push_front((Int_t)vec->Y());
    
    trksassocok = trksassocok && vec->X()>=0 && vec->X()==vec->Y();
  }
  assoctrklist.sort();
  assoctrklist.unique();
  
  // 2a. get the list of tracks which pass the TPCOnly cuts and have hits on
  // SPD layers 1 and 2 and in the TPC -> tpconlylist    
  tpconlylist.clear();
  Int_t ntrk = ev->GetnTracks();
  for (Int_t jj=0; jj<ntrk; jj++) {
    trind = ev->GetTrack(jj)->GetTrackindex();
    
    // only tracks which pass TPCOnly cuts are considered
    trkstat = ev->GetTrack(jj)->GetTrackStatus();
    if (trkstat & AliCEPBase::kTTAccITSTPC) {
      ITSncls = ev->GetTrack(jj)->GetITSncls();
      if (
        (ITSncls & (1<<0)) && 
        (ITSncls & (1<<1)) && 
        ev->GetTrack(jj)->GetTPCncls()>minnTPCclus )
        tpconlylist.push_front(trind);
    }
  }
  tpconlylist.sort();

  // 2b. now check that all tracks in assoctrklist are also in tpconlylist
  assoctrksok = kTRUE;
  
  std::list<Int_t>::iterator it;
  for (it=assoctrklist.begin(); it!=assoctrklist.end(); it++) {
    assoctrksok = assoctrksok &&
      std::find(tpconlylist.begin(),tpconlylist.end(),*it)!=tpconlylist.end();
  }
  
  // 3. the number of singles must be <= maxnSingle
  singleok = ev->GetnSingles()<=maxnSingle;
  
  // 4. the number of clusters on layers 1 and 2 is not larger than
  // (the number of tracklets + maxdnclu)
  SPDhitok = kTRUE;
  for (Int_t jj=0; jj<2; jj++)
    SPDhitok = SPDhitok && (ev->GetnITSCluster(jj)<=(ntrl+maxdnclu));
  
  // 5. the number of SPD fired chips of layer 1 and layer 2 is not larger
  // than (the number of tracklets + maxdnfchips)
  SPDfchipsok = kTRUE;
  for (Int_t jj=0; jj<2; jj++) SPDfchipsok =
    SPDfchipsok && (ev->GetnFiredChips(jj)<=(ntrl+maxdnfchips));
  
  // 6. the number of SPD fired FastOR chips of layer 1 and layer 2 is not
  // larger than (the number of tracklets + maxdnfFOchips)
  SPDfFOchipsok = kTRUE;
  for (Int_t jj=2; jj<4; jj++) SPDfFOchipsok =
    SPDfFOchipsok && (ev->GetnFiredChips(jj)<=(ntrl+maxdnfFOchips));

  // 7. check the vertex position
  vtxok = kTRUE;
  
  UInt_t vtxtype = ev->GetVtxType();
  TVector3 vtxpos = ev->GetVtxPos();
  if (
    (vtxtype & AliCEPBase::kVtxErrRes) ||
    (vtxtype & AliCEPBase::kVtxErrRes) ||
    (vtxtype & AliCEPBase::kVtxErrDif) ) vtxok = kFALSE;
  
  
  goodevent = kTRUE;
  if (tomatch & BIT(0)) goodevent = goodevent && trksassocok;
  if (tomatch & BIT(1)) goodevent = goodevent && assoctrksok;
  if (tomatch & BIT(2)) goodevent = goodevent && singleok;
  if (tomatch & BIT(3)) goodevent = goodevent && SPDhitok;
  if (tomatch & BIT(4)) goodevent = goodevent && SPDfchipsok;
  if (tomatch & BIT(5)) goodevent = goodevent && SPDfFOchipsok;
  if (tomatch & BIT(6)) goodevent = goodevent && vtxok;
    
  // detailed printout of situation
  if (goodevent && verbose) {
    
    // loop over tracks
    printf("\ntracks");
    for (Int_t jj=0; jj<ntrk; jj++) {
      trind = ev->GetTrack(jj)->GetTrackindex();
      trkstat = ev->GetTrack(jj)->GetTrackStatus();
      printf(" %i/%i",trind,(trkstat & AliCEPBase::kTTAccITSTPC));
    }
    printf("\n");

    printf("tpconlylist[%i]",tpconlylist.size());
    for (std::list<Int_t>::iterator it=tpconlylist.begin(); it != tpconlylist.end(); ++it)
      std::cout << ' ' << *it;
    printf("\n");
    printf("assoctrklist[%i]",assoctrklist.size());
    for (std::list<Int_t>::iterator it=assoctrklist.begin(); it != assoctrklist.end(); ++it)
      std::cout << ' ' << *it;
    printf("\n");
    printf("lists are identical %i\n",assoctrklist==tpconlylist);

    printf("singles %i, tracklets %i:",ev->GetnSingles(),ntrl);
    for (Int_t jj=0; jj<ntrl; jj++) {
      vec->Set(-1.,-1.);
      if (jj<trl2tr->GetEntries()) {
        vectmp = (TVector2*) trl2tr->At(jj);
        vec->Set(vectmp->X(),vectmp->Y());
      }
      printf(" [%i] %i/%i",jj,(Int_t)vec->X(),(Int_t)vec->Y());
    }
    printf("\n");

    printf("ITS clusters\n");
    for (Int_t jj=2; jj<6; jj++) printf(" [%i] %i",jj,ev->GetnITSCluster(jj));
    printf("\n");
    for (Int_t jj=0; jj<2; jj++) printf(" [%i] %i",jj,ev->GetnITSCluster(jj));
    printf("\n");
    printf("Fired chips\n");
    for (Int_t jj=0; jj<2; jj++) printf(" [%i] %i",jj,ev->GetnFiredChips(jj));
    printf("\n");
    for (Int_t jj=2; jj<4; jj++) printf(" [%i] %i",jj-2,ev->GetnFiredChips(jj));
    printf("\n");

    printf("tracks %i\n",ntrk);
    for (Int_t jj=0; jj<ntrk; jj++) {
      trind = ev->GetTrack(jj)->GetTrackindex();
      ITSncls = ev->GetTrack(jj)->GetITSncls();
      
      printf("%4i/%i/%i/%4i\n",trind,
        (ITSncls&(1<<0))==1,(ITSncls&(1<<1))==2,
        ev->GetTrack(jj)->GetTPCncls());
    }
    
  }

  return goodevent;
  
}

// ----------------------------------------------------------------------------
