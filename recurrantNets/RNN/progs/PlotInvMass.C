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

void PlotInvMass(TString fname)
{
    // output has the same name (but with pdf ending)
    TString output_file = fname;
    // remove the .root at the end of the filename ".root" = 5 characters at the end
    output_file.Remove(output_file.Length()-5, 5);
    // now append .pdf
    output_file += ".pdf";


    TFile* file = TFile::Open(fname.Data());
    if (!file) { std::cout << "<E> File path not found\n"; gSystem->Exit(1);  }
    TCanvas* c = new TCanvas("InvMass","",800,600);

    // get some info from the file name for the legend 
    /* if (fname.Contains("LHC")) { leg_string_1 = "Full recon"; leg_string_2 = "Feed down"; } */
    TString leg_string_1 = "Full recon"; 
    TString leg_string_2 = "Feed down";
    TString leg_string_sum = "Event";
    TString title;
    if (fname.Contains("AD")) title = "!AD";
    if (fname.Contains("V0")) title = "!V0";
    if (fname.Contains("checkSPD-hard")) title = "checkSPD-hard";
    if (fname.Contains("checkSPD") && fname.Contains("0")) title = "loose checkSPD maxnSingles0";
    if (fname.Contains("checkSPD") && fname.Contains("1")) title = "loose checkSPD maxnSingles1";
    if (fname.Contains("checkSPD") && fname.Contains("2")) title = "loose checkSPD maxnSingles2";
    else title = "ML-cut";

    // file extensions
    // 1: loop through one of the files to get all hists
    SetStyle();
    TH1F* h_fd        = (TH1F*)file->Get("invar_mass_feed_down");
    TH1F* h_fullrecon = (TH1F*)file->Get("invar_mass_full_recon");
    if (!h_fd->InheritsFrom("TH1") || !h_fullrecon->InheritsFrom("TH1")) { std::cout << "<E> No histogram found in file\n"; gSystem->Exit(1);  }

    TH1F* h_sum = new TH1F("sum hist", title.Data(), h_fd->GetXaxis()->GetNbins(), h_fd->GetXaxis()->GetXmin(), h_fd->GetXaxis()->GetXmax());
    h_sum->Add(h_fd);
    h_sum->Add(h_fullrecon);

    Double_t norm = 0.;
    Double_t integral_sum       = h_sum->Integral(1,h_sum->GetSize()-2); 
    Double_t integral_fd        = h_fd->Integral(1,h_fd->GetSize()-2); 
    Double_t integral_fullrecon = h_fullrecon->Integral(1,h_fullrecon->GetSize()-2); 
    TString n_sum, part_fd, part_fullrecon;
    n_sum.Form("%i", Int_t(integral_sum));
    part_fd.Form("%.2f", (integral_fd/integral_sum)*100.);
    part_fullrecon.Form("%.2f", (integral_fullrecon/integral_sum)*100.);

    h_fd->Sumw2();
    h_fullrecon->Sumw2();
    h_sum->Sumw2();

    h_fd->SetLineColor(kRed);
    h_fullrecon->SetLineColor(kGreen+1);
    h_sum->GetXaxis()->SetTitle("M_{inv} [GeV]");
    h_sum->GetYaxis()->SetTitle("N_{evts}");
    /* h_sum->SetTitle(title); */

    h_sum->Draw("P");
    h_fd->Draw("sameP");
    h_fullrecon->Draw("sameP");

    h_fd->SetMarkerStyle(kFullTriangleDown);
    h_fullrecon->SetMarkerStyle(kFullTriangleUp);
    h_sum->SetMarkerStyle(kFullCircle);

    h_fd->SetMarkerSize(1.5);
    h_fullrecon->SetMarkerSize(1.5);
    h_sum->SetMarkerSize(1.5);

    h_fd->SetMarkerColor(kRed);
    h_fullrecon->SetMarkerColor(kGreen+1);
    h_sum->SetMarkerColor(kBlack);


    /* rp->SetLeftMargin(0.15); */
    /* rp->GetUpperRefYaxis()->SetLabelSize(0.045); */
    /* rp->GetUpperRefYaxis()->SetLabelOffset(0.01); */
    /* rp->GetUpperRefYaxis()->SetTitle("entries"); */
    /* rp->GetUpperRefYaxis()->SetTitleSize(0.05); */
    /* rp->GetUpperRefYaxis()->SetTitleOffset(1.2); */

    /* rp->GetLowYaxis()->SetNdivisions(505); */
    /* rp->GetLowerRefYaxis()->SetTitle("ratio"); */
    /* rp->GetLowerRefYaxis()->SetTitleSize(0.05); */
    /* rp->GetLowerRefYaxis()->SetTitleOffset(1.2); */
    /* rp->GetLowerRefYaxis()->SetRangeUser(0.5,1.5); */

    /* rp->SetLowBottomMargin(0.40); */
    /* rp->GetLowerRefXaxis()->SetLabelOffset(0.01); */
    /* /1* rp->GetLowerRefXaxis()->SetTitleOffset(1.05); *1/ */
    /* /1* rp->GetLowerRefXaxis()->SetTitleSize(1.2); *1/ */

    /* rp->SetSeparationMargin(0.01); */

    TLegend* leg = new TLegend(0.591336,0.650505,0.922756,0.849495);
    leg->AddEntry(h_sum, (n_sum+" events").Data(),"p");
    leg->AddEntry(h_fd, (leg_string_2+": "+part_fd+"%").Data(),"p");
    leg->AddEntry(h_fullrecon, (leg_string_1+": "+part_fullrecon+"%").Data(),"p");
    leg->SetFillStyle(0);
    leg->Draw();

    c->Update();
    c->SaveAs(output_file.Data());

    /* std::cout << "\nSaving output in " << output_file.Data() << std::endl; */
    std::cout << "\nEverything successful!" << std::endl;
}
 
