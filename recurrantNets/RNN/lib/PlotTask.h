#ifndef PlotTask_H
#define PlotTask_H

#include "TList.h"
#include "TString.h"
#include "TH1F.h"

class PlotTask 
{
  public:
    // Constructor
                        PlotTask(TString fname, TString option="BG");
    // Destructor
                        ~PlotTask();
    // Setters
    void                SetBatch(Bool_t sb) { fSetBatch = sb; }
    void                SetLog(Bool_t lp) { fLogPlot = lp; }
    void                SetTitleSize(Int_t size) { fTitleSize = size; }
    void                SetLableSize(Int_t size) { fLableSize = size; }
    void                SetPlotTextSize(Int_t size) { fPlotTextSize = size; }
    void                SetLegendTextSize(Int_t size) { fLegendTextSize = size; }

    void                ResetSizes();

    // functions plotting a histogram and saving it in 
    void                PlotAdd(TString hname1, TString hname2="", TString hname3="", 
                                TString hname4="", TString hname5="") const;
    void                PlotRatio(TString hname1, TString hname2, TString hname3="", 
                                  TString hname4="", TString hname5="") const;
    void                AddHists(TString finalName, TString hname1, TString hname2,
                                 TString hname3="", TString hname4="");
 
  private:
    // if the y axis should be logarithmic
    Bool_t              fLogPlot;        
    // if the plot should just be saved or displayed
    Bool_t              fSetBatch;
    // the list of histograms
    TList               *fHistList;
    // the basename for the output
    TString             fOutFileBaseName;

    // plot specific variables
    Int_t               fTitleSize;
    Int_t               fLableSize;
    Int_t               fPlotTextSize;
    Int_t               fLegendTextSize;

    // private functions returning the canvas
    TCanvas*            PlotAddHists(TH1F* hist1, TH1F* h_2=0x0, 
                                     TH1F* h_3=0x0, TH1F* h_4=0x0) const;
    TCanvas*            PlotHist(TH1F* hist1) const;
    TCanvas*            rp(TH1F* main_hist, TH1F* h2, TH1F* h3=0x0, 
                           TH1F* h4=0x0, TH1F* h5=0x0) const;
    
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
