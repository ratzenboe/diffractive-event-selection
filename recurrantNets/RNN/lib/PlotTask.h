#ifndef PlotTask_H
#define PlotTask_H

#include "TList.h"
#include "TString.h"
#include "TH1F.h"

class PlotTask 
{
  public:
    // Constructor
                        PlotTask(TString fname, TString option="");
    // Destructor
                        ~PlotTask();

    // add histograms from a diffent file
    void                AddFile(TString filename, TString name_addon="", TString option="");

    // Setters
    void                SetLog(Bool_t lp) { fLogPlot = lp; }
    // Sizes
    void                SetTitleSize(Int_t size) { fTitleSize = size; }
    void                SetLableSize(Int_t size) { fLableSize = size; }
    void                SetPlotTextSize(Int_t size) { fPlotTextSize = size; }
    void                SetLegendTextSize(Int_t size) { fLegendTextSize = size; }
    // axis range
    void                SetAxisRange(Double_t xmin, Double_t xmax) {fAxisMin=xmin; fAxisMax=xmax; fSetRange=kTRUE;}
    // Positional & text setters
    void                SetLegendPos(Double_t xmin, Double_t ymin, Double_t xmax, Double_t ymax);
    void                SetTextPos(Double_t x, Double_t y) { fTextX = x; fTextY = y; }
    void                SetTextString(TString txtstr) { fTextString = txtstr; }

    void                SetStdColors(Int_t c1=kBlack, Int_t c2=6, Int_t c3=8, 
                                     Int_t c4=kBlue, Int_t c5=kYellow);
    void                SetSigBgColors(Int_t c1=kGreen+1, Int_t c2=kRed+1);
    void                SetXaxisText(TString txt) { fXaxisText = txt; }
    void                SetYaxisText(TString txt) { fYaxisText = txt; }

    void                ResetSizes();
    void                ResetLegendPos();
    void                ResetTextPos() { fTextX = -999.; fTextY = -999.; }
    void                ResetTextString() { fTextString = ""; }
    void                ResetColors();
    void                ResetAxisText();
    // swich functions
    void                ToggleDisplayCanvas() { fSetBatch = !fSetBatch; }
    void                ToggleAutoRange() { fSetRange = !fSetRange; }

    // functions plotting a histogram and saving it in 
    void                PlotAdd(TString hname1, TString hname2="", TString hname3="", 
                                TString hname4="", TString hname5="") const;
    void                PlotRatio(TString hname1, TString hname2, TString hname3="", 
                                  TString hname4="", TString hname5="") const;
    void                PlotSigBg(TString hnameSig, TString hnameBg, Bool_t kNorm=kFALSE) const;
    void                PlotSignificance(TString hnameSig, TString hnameBg) const;
    // add histograms together creating a new one with name&title finalName
    void                AddHists(TString finalName, TString hname1, TString hname2,
                                 TString hname3="", TString hname4="");
    void                LikeSignHist(TString hLSplus, TString hLSminus);
    // calculate reduction rate from horiginal to hreduced and save new histogram under newname:
    void                ReductionRateHist(TString horiginal, TString hreduced, TString newname);

    // print out the various histograms stored in fHistlist
    void                PrintHists() const;
    // Change title of a histogram
    void                ChangeTitleOfHist(TString hname, TString newTitle);

  private:
    // if the y axis should be logarithmic
    Bool_t              fLogPlot;        
    // if the plot should just be saved or displayed
    Bool_t              fSetBatch;
    // if range should be set manually
    Bool_t              fSetRange;
    // the list of histograms
    TList               *fHistList;
    // the basename for the output
    TString             fOutFileBaseName;

    // plot specific variables
    Int_t               fTitleSize;
    Int_t               fLableSize;
    Int_t               fPlotTextSize;
    Int_t               fLegendTextSize;

    // axis range
    Double_t            fAxisMin;
    Double_t            fAxisMax;
    // legend position
    Double_t            fLegXmin;
    Double_t            fLegXmax;
    Double_t            fLegYmin;
    Double_t            fLegYmax;
    // text position
    Double_t            fTextX;
    Double_t            fTextY;
    // text string
    TString             fTextString;
    // Colors 
    Int_t               fStdColors[5];
    Int_t               fSigBgColors[2];
    // X-y labels
    TString             fXaxisText;
    TString             fYaxisText;

    // private functions returning the canvas
    TCanvas*            PlotAddHists(TH1F* hist1, TH1F* h_2=0x0, 
                                     TH1F* h_3=0x0, TH1F* h_4=0x0, TH1F* h_5=0x0) const;
    TCanvas*            PlotHist(TH1F* hist1) const;
    TCanvas*            rp(TH1F* main_hist, TH1F* h2, TH1F* h3=0x0, 
                           TH1F* h4=0x0, TH1F* h5=0x0) const;
    TCanvas*            SigBg(TH1F* h_sig, TH1F* h_bg) const;
    TCanvas*            Significance(TH1F* h_sig, TH1F* h_bg) const;

    // get the title of a histogram
    TString             Title(TH1F* hist) const;
    // scale histogram to other histogram
    TH1F*               ScaleHist(TH1F* h_toScale, TH1F* h_main) const;
    TH1F*               ScaleHist(TH1F* h_toScale, Double_t finalAUC) const;

    // figure out where to place the alice simulation text
    void                RelativeTextPosition(Double_t &x, Double_t &y) const;

    // set the alice plotting style
    void                SetStyle(Bool_t graypalette=kFALSE) const;

    ClassDef(PlotTask, 1)     // CEP raw track buffer
};

#endif
