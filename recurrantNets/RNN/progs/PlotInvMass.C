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
    gStyle->SetFrameLineWidth(1.5);
    gStyle->SetFrameFillColor(kWhite);
    gStyle->SetPadColor(10);
    gStyle->SetPadTickX(1);
    gStyle->SetPadTickY(1);
    gStyle->SetPadBottomMargin(0.14);
    gStyle->SetPadLeftMargin(0.14);
    gStyle->SetHistLineWidth(1);
    /* gStyle->SetHistLineColor(kRed); */
    gStyle->SetFuncWidth(2);
    gStyle->SetFuncColor(kGreen);
    gStyle->SetLineWidth(2);
    /* gStyle->SetMarkerStyle(20); */
    /* gStyle->SetMarkerSize(1); */
    gStyle->SetLabelSize(0.045,"xyz");
    gStyle->SetLabelOffset(0.01,"y");
    gStyle->SetLabelOffset(0.01,"x");
    gStyle->SetLabelColor(kBlack,"xyz");
    gStyle->SetTitleSize(0.05,"xy");
    gStyle->SetTitleOffset(1.25,"y");
    gStyle->SetTitleOffset(1.2,"x");
    /* gStyle->SetTitleFillColor(kWhite); */
    gStyle->SetTextSizePixels(26);
    gStyle->SetTextFont(42);
    gStyle->SetTickLength(0.04,"X"); gStyle->SetTickLength(0.04,"Y");
    gStyle->SetLegendBorderSize(0);
    gStyle->SetLegendFillColor(kWhite);
    //  gStyle->SetFillColor(kWhite);
    gStyle->SetLegendFont(42);

    std::cout << "ALICE style set." << std::endl;
}

void PlotInvMass(TString fname, TString fname2, TString title1, TString title2)
{
    // output has the same name (but with pdf ending)
    TString output_file = fname;
    // remove the .root at the end of the filename ".root" = 5 characters at the end
    output_file.Remove(output_file.Length()-5, 5);
    // now append .pdf
    output_file += ".pdf";


    TFile* file = TFile::Open(fname.Data());
    if (!file) { std::cout << "<E> File path not found\n"; gSystem->Exit(1);  }
    TFile* file2 = TFile::Open(fname2.Data());
    if (!file2) { std::cout << "<E> File path not found\n"; gSystem->Exit(1);  }
    TCanvas* c = new TCanvas("InvMass","",800,600);

    // 1: loop through one of the files to get all hists
    SetStyle();
    TH1F* h_fd   = (TH1F*)file->Get("invar_mass_feed_down");
    TH1F* h_fd_2 = (TH1F*)file2->Get("invar_mass_feed_down");
    if (!h_fd->InheritsFrom("TH1") || !h_fd_2->InheritsFrom("TH1")) { std::cout << "<E> No histogram found in file\n"; gSystem->Exit(1);  }

    Double_t integral_fd   = h_fd->Integral(1,h_fd->GetSize()-2); 
    Double_t integral_fd_2 = h_fd_2->Integral(1,h_fd_2->GetSize()-2); 
    TString n_fd, n_fd_2, n_reduction;
    n_reduction.Form("%i", Int_t(integral_fd/ integral_fd_2));
    n_fd.Form("%i", Int_t(integral_fd));
    n_fd_2.Form("%i", Int_t(integral_fd_2));

    h_fd->Sumw2();
    h_fd_2->Sumw2();

    h_fd->SetLineColor(kRed);
    h_fd_2->SetLineColor(kGreen+1);
    h_fd->GetXaxis()->SetTitle("M_{#pi#pi} [GeV]");
    h_fd->GetYaxis()->SetTitle("N_{evts}");
    /* h_sum->SetTitle(title); */

    h_fd->Draw("P");
    h_fd_2->Draw("sameP");

    h_fd->SetMarkerStyle(kFullCircle);
    h_fd_2->SetMarkerStyle(kFullRectangle);

    h_fd->SetMarkerSize(1.1);
    h_fd_2->SetMarkerSize(1.1);

    h_fd->SetMarkerColor(kRed);
    h_fd_2->SetMarkerColor(kGreen+1);

    TLegend* leg = new TLegend(0.591336,0.650505,0.922756,0.849495);
    leg->AddHeader(("Reduction: "+n_reduction).Data());
    leg->AddEntry(h_fd, (title1+" :"n_fd).Data(),"p");
    leg->AddEntry(h_fd_2, (title2+" :"n_fd_2).Data(),"p");
    leg->SetFillStyle(0);
    leg->Draw();

    c->Update();
    c->SaveAs(output_file.Data());

    /* std::cout << "\nSaving output in " << output_file.Data() << std::endl; */
    std::cout << "\nEverything successful!" << std::endl;
}
 
