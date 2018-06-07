/// ~~~{.C}

#include <iostream>
#include <stdlib.h> 
#include <sys/stat.h>
#include <time.h>

#include <TROOT.h>
#include <TSystem.h>
#include <TObjArray.h>
#include <TDirectory.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <TList.h>
#include <TString.h>
#include <TFile.h>
/* #include <TRatioPlot.h> */

void SetStyle(Bool_t graypalette=kFALSE) 
{
    gStyle->Reset("Plain");
    gStyle->SetOptTitle(0);
    gStyle->SetOptStat(0);
    if(graypalette) gStyle->SetPalette(8,0);
    else gStyle->SetPalette(57);
    gStyle->SetCanvasColor(10);
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetFrameLineWidth(1);
    gStyle->SetFrameFillColor(kWhite);
    gStyle->SetPadColor(10);
    gStyle->SetPadTickX(1);
    gStyle->SetPadTickY(1);
    gStyle->SetPadBottomMargin(0.15);
    gStyle->SetPadLeftMargin(0.15);
    gStyle->SetHistLineWidth(1);
    gStyle->SetHistLineColor(kRed);
    gStyle->SetFuncWidth(2);
    gStyle->SetFuncColor(kGreen);
    gStyle->SetLineWidth(2);
    // gStyle->SetMarkerStyle(20);
    gStyle->SetMarkerSize(1);
    /* gStyle->SetLabelSize(0.045,"xyz"); */
    /* gStyle->SetLabelOffset(0.01,"y"); */
    /* gStyle->SetLabelOffset(0.01,"x"); */
    /* gStyle->SetLabelColor(kBlack,"xyz"); */
    gStyle->SetTitleSize(0.05,"x");
    /* gStyle->SetTitleOffset(1.25,"y"); */
    /* gStyle->SetTitleOffset(1.2,"x"); */
    /* gStyle->SetTitleFillColor(kWhite); */
    /* /1* gStyle->SetTextSizePixels(26); *1/ */
    /* gStyle->SetTextFont(42); */
    /* //  gStyle->SetTickLength(0.04,"X"); gStyle->SetTickLength(0.04,"Y"); */
    gStyle->SetLegendBorderSize(0);
    gStyle->SetLegendFillColor(kWhite);
    //  gStyle->SetFillColor(kWhite);
    gStyle->SetLegendFont(42);

    std::cout << "ALICE style set." << std::endl;
}

void CreateRatioPlots(TString fname_1, TString fname_2, TString outpath)
{
    if ((outpath != "") && !outpath.EndsWith("/")) outpath+="/";
    printf("outpath: %s\n", outpath.Data());

    TFile* file_1 = TFile::Open(fname_1);
    TFile* file_2 = TFile::Open(fname_2);
    if (!file_1 || !file_2) { std::cout << "<E> File paths not found\n"; gSystem->Exit(1);  }

    // get some info from the file name for the legend 
    TString leg_string_1, leg_string_2;
    /* if (fname_1.Contains("LHC")) { leg_string_1 = "LHC data"; leg_string_2 = "CEP simulation"; } */
    /* if (fname_1.Contains("LHC")) { leg_string_1 = "LHC !V0"; leg_string_2 = "LHC checkSPD"; } */
    /* else { leg_string_2 = "LHC !V0"; leg_string_1 = "LHC checkSPD"; } */
    /* else { leg_string_1 = "CEP simulation"; leg_string_2 = "LHC data"; } */
    /* if (fname_1.Contains("99")) { leg_string_1 = "3+ tracks bg"; leg_string_2 = "Feed Down"; } */
    /* else { leg_string_1 = "Feed down"; leg_string_2 = "3+ tracks bg"; } */
    /* if (fname_1.Contains("1")) { leg_string_1 = "Signal"; leg_string_2 = "Feed Down"; } */
    /* else { leg_string_1 = "Feed down"; leg_string_2 = "Signal"; } */
    if (fname_1.Contains("with")) { leg_string_1 = "w/ correction"; leg_string_2 = "no correction"; }
    else { leg_string_1 = "no correction"; leg_string_2 = "w/ correction"; }
      
    // file extensions
    // 1: loop through one of the files to get all hists
    SetStyle();
    TCanvas* c = new TCanvas("RatioPlot_","",800,600);
    Double_t norm = 0.;
    TList* list_of_histnames = file_1->GetListOfKeys();
    if (!list_of_histnames) { std::cout << "<E> No keys found in file\n"; gSystem->Exit(1);  }
    TIter next(list_of_histnames);
    TKey* key;
    TObject* obj;
    while ( (key = (TKey*)next()) ) 
    {
        obj = (TH1F*)key->ReadObj();
        if (!obj->InheritsFrom("TH1")) { std::cout << "The list contains NON-TH1 objects!\n"; }
        else std::cout << "Histo name: " << obj->GetName() << std::endl;
        // at first we clone the histogram to which we then add the remaining hists 
        // the Remove(0,5) removes the "full_"-chars in front of the histogram name
        /* TH1F* clone_hist_1 = (TH1F*)obj->Clone((TString(obj->GetName()).Remove(0,5)).Data()); */
        TH1F* clone_hist_1 = (TH1F*)obj->Clone((TString(obj->GetName())+" ").Data());
        /* TH1F* clone_hist_1 = (TH1F*)obj->Clone((TString(obj->GetName())).Data()); */
        /* TH1F* clone_hist_1 = (TH1F*)obj->Clone("M_{#pi#pi} [GeV]"); */
        TH1F* clone_hist_2 = (TH1F*)((TH1F*)file_2->Get(obj->GetName()))->Clone((TString(obj->GetName())+"_2").Data());
        if (clone_hist_1->GetEntries()==0 || clone_hist_2->GetEntries()==0) continue;
        Double_t integral_1 = clone_hist_1->Integral(1,clone_hist_1->GetSize()-2); 
        Double_t integral_2 = clone_hist_2->Integral(1,clone_hist_2->GetSize()-2); 
        clone_hist_1->Sumw2();
        if (integral_1>integral_2) { norm = integral_2 / integral_1; clone_hist_1->Scale(norm); }
        else { norm = integral_1 / integral_2; clone_hist_2->Scale(norm); }

        clone_hist_2->SetLineColor(kRed);
        clone_hist_1->SetLineWidth(2);
        clone_hist_2->SetLineWidth(2);
        clone_hist_1->GetXaxis()->SetTitle(clone_hist_1->GetName());

        if (!clone_hist_1->InheritsFrom("TH1") || !clone_hist_2->InheritsFrom("TH1")) { std::cout << "<E> No histogram found in file\n"; gSystem->Exit(1);  }

        TRatioPlot* rp = new TRatioPlot(clone_hist_1, clone_hist_2);
        rp->Draw();
        rp->GetUpperPad()->SetLogy();

        rp->SetLeftMargin(0.15);
        rp->GetUpperRefYaxis()->SetLabelSize(0.045);
        rp->GetUpperRefYaxis()->SetLabelOffset(0.01);
        rp->GetUpperRefYaxis()->SetTitle("entries");
        rp->GetUpperRefYaxis()->SetTitleSize(0.05);
        rp->GetUpperRefYaxis()->SetTitleOffset(1.2);
        Double_t minimum, maximum;
        minimum = TMath::Min(clone_hist_1->GetBinCenter(clone_hist_1->FindFirstBinAbove()), clone_hist_2->GetBinCenter(clone_hist_2->FindFirstBinAbove()));
        maximum = TMath::Min(clone_hist_1->GetBinCenter(clone_hist_1->FindLastBinAbove()), clone_hist_2->GetBinCenter(clone_hist_2->FindLastBinAbove()));
        rp->GetUpperRefXaxis()->SetRangeUser(minimum*0.9, maximum*1.1);

        rp->GetLowYaxis()->SetNdivisions(505);
        rp->GetLowerRefYaxis()->SetTitle("ratio");
        rp->GetLowerRefYaxis()->SetTitleSize(0.05);
        rp->GetLowerRefYaxis()->SetTitleOffset(1.2);
        rp->GetLowerRefYaxis()->SetRangeUser(0.5,1.5);
        rp->GetLowerRefXaxis()->SetRangeUser(minimum*0.9, maximum*1.1);

        rp->SetLowBottomMargin(0.40);
        rp->GetLowerRefXaxis()->SetLabelOffset(0.01);
        /* rp->GetLowerRefXaxis()->SetTitleOffset(1.05); */
        /* rp->GetLowerRefXaxis()->SetTitleSize(1.2); */

        rp->SetSeparationMargin(0.01);

        TLegend* leg = new TLegend(0.66, 0.76, 0.89, 0.89);
        /* leg->SetHeader(("Factor: "+(TString)Form("%.1f",norm)).Data()); */
        leg->AddEntry(clone_hist_1, leg_string_1, "l");
        leg->AddEntry(clone_hist_2, leg_string_2, "l");
        leg->SetFillStyle(0);
        leg->Draw();

        c->Update();
        c->SaveAs((outpath+TString(clone_hist_1->GetName()).Chop()+".pdf").Data());
        /* c->SaveAs("ratioplot.pdf"); */
    }

    std::cout << "\nEverything successful!" << std::endl;
}
 
