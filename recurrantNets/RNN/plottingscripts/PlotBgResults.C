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
    gStyle->SetHistLineWidth(2);
    /* gStyle->SetHistLineColor(kRed); */
    gStyle->SetFuncWidth(2);
    gStyle->SetFuncColor(kGreen);
    gStyle->SetLineWidth(2);
    gStyle->SetMarkerStyle(20);
    gStyle->SetMarkerSize(1);
    gStyle->SetLabelSize(0.045,"xyz");
    gStyle->SetLabelOffset(0.01,"y");
    gStyle->SetLabelOffset(0.01,"x");
    gStyle->SetLabelColor(kBlack,"xyz");
    gStyle->SetTitleSize(0.06,"xy");
    gStyle->SetTitleOffset(1.25,"y");
    gStyle->SetTitleOffset(1.2,"x");
    /* gStyle->SetTitleFillColor(kWhite); */
    gStyle->SetTextSizePixels(24);
    gStyle->SetTextFont(42);
    gStyle->SetTickLength(0.04,"X"); gStyle->SetTickLength(0.04,"Y");
    gStyle->SetLegendBorderSize(0);
    gStyle->SetLegendFillColor(kWhite);
    //  gStyle->SetFillColor(kWhite);
    gStyle->SetLegendFont(42);

    printf("ALICE style set.\n");
}

void Plot(TString fname, TString hname1, TString hname2="", TString hname3="", TString hname4="")
{
    // output has the same name (but with pdf ending)
    TString output_file = fname;
    // remove the .root at the end of the filename ".root" = 5 characters at the end
    output_file.Remove(output_file.Length()-5, 5);
    // now append .pdf
    output_file += ".pdf";

    // set the alice plot-style
    SetStyle();

    // open the file
    TFile* file = TFile::Open(fname.Data());
    if (!file) { printf("<E> File %s not found!\n", fname.Data()); gSystem->Exit(1);  }
    TDirectory* dir = file->GetDirectory("BGTask");
    TList* lst = (TList*)dir->Get("BGOutputContainer");
    if (!lst) { printf("<E> List not found in file %s\n", fname.Data()); gSystem->Exit(1);  }
    // create a plotting canvas

    // 1: loop through one of the files to get all hists
    TH1F* h_1 = (TH1F*)lst->FindObject(hname1);
    TH1F *h_2(0x0), *h_3(0x0), *h_4(0x0);
    /* hist->Add((TH1F*)file->Get(hname1)); */
    if (hname2!="") h_2 = (TH1F*)lst->FindObject(hname2);
    if (hname3!="") h_3 = (TH1F*)lst->FindObject(hname3);
    if (hname4!="") h_4 = (TH1F*)lst->FindObject(hname4);
    if (!h_1 || !h_1->InheritsFrom("TH1")) { printf("<E> No histogram found named %s\n", hname1.Data()); gSystem->Exit(1);  }
    if (h_2 && !h_2->InheritsFrom("TH1"))  { printf("<E> No histogram found named %s\n", hname2.Data()); gSystem->Exit(1);  }
    if (h_3 && !h_3->InheritsFrom("TH1"))  { printf("<E> No histogram found named %s\n", hname3.Data()); gSystem->Exit(1);  }
    if (h_4 && !h_4->InheritsFrom("TH1"))  { printf("<E> No histogram found named %s\n", hname4.Data()); gSystem->Exit(1);  }

    /* Double_t integral_1 = h_1->Integral(1,h_1->GetSize()-2); */ 
    /* Double_t integral_2(-1), integral_3(-1), integral_4(-1); */
    /* if (h_2) integral_2 = h_2->Integral(1,h_2->GetSize()-2); */ 
    /* if (h_3) integral_3 = h_3->Integral(1,h_3->GetSize()-2); */ 
    /* if (h_4) integral_4 = h_4->Integral(1,h_4->GetSize()-2); */ 

    /* TString n_1, n_2, n_3, n_4; */
    /* n_1.Form("%i", Int_t(integral_1)); */
    /* n_2.Form("%i", Int_t(integral_2)); */
    /* n_3.Form("%i", Int_t(integral_3)); */
    /* n_4.Form("%i", Int_t(integral_4)); */

    /* h_1->Sumw2(); */
    /* if (h_2) h_2->Sumw2(); */
    /* if (h_3) h_3->Sumw2(); */
    /* if (h_4) h_4->Sumw2(); */

    if (h_1 && !h_2 && !h_3 && !h_4) PlotHist(h_1, output_file);
    if (h_2 || h_3 || h_4) PlotAddHists(h_1, output_file, h_2, h_3, h_4);
}
 
void PlotHist(TH1F* hist, TString output_file)
{
    // go into batch mode where canvases are not drawn:
    gROOT->SetBatch(kTRUE);

    TCanvas* canv = new TCanvas("InvMass","",1200,820);

    Double_t integral = hist->Integral(1,hist->GetSize()-2); 
    TString n;
    n.Form("%i", Int_t(integral));

    hist->Sumw2();

    hist->SetLineColor(kBlue);
    hist->SetLineWidth(2);
    hist->SetMarkerStyle(20);
    hist->SetMarkerSize(1.4);
    hist->SetMarkerColor(kBlue);

    hist->GetXaxis()->SetTitle("m_{#pi#pi} (GeV/c^{2})");
    hist->GetYaxis()->SetTitle("Counts / (0.03 GeV/c^{2})");
    // axis labels have to be set seperately,
    // gStyle is somehow set to default for the next adjustments
    hist->GetXaxis()->SetTitleOffset(1.1);
    hist->GetYaxis()->SetTitleOffset(1.15);
    hist->GetYaxis()->SetTitleSize(0.06);
    hist->GetXaxis()->SetTitleSize(0.06);
    hist->GetXaxis()->SetTitleFont(42);
    hist->GetYaxis()->SetTitleFont(42);
    hist->GetXaxis()->SetLabelSize(0.055);
    hist->GetYaxis()->SetLabelSize(0.055);
    hist->GetXaxis()->SetLabelFont(42);
    hist->GetYaxis()->SetLabelFont(42);

    hist->SetAxisRange(0., 2.5,"X");

    hist->Draw();
    /* hist->SetMarkerStyle(k); */
    /* hist->SetMarkerSize(1.2); */
    /* hist->SetMarkerColor(kBlack); */
    TLatex tex;
    tex.DrawLatex(1.11086, 622.392, "#splitline{#splitline{ALICE simulation, this work}{Pythia-8 MBR (#varepsilon=0.08)}}{#sqrt{s}=13 TeV}");

    TLegend* leg = new TLegend(0.575377, 0.522727, 0.908291, 0.613636);
    leg->AddEntry(hist, (Title(hist)).Data(), "pe");
    /* leg->SetTextSize(0.04); */
    leg->SetFillStyle(0);
    leg->Draw();

    canv->Update();
    canv->SaveAs(output_file.Data());

    printf("Saving output in %s\n", output_file.Data());

    /* delete canv; */
}

void PlotAddHists(TH1F* hist1, TString output_file, TH1F* h_2=0x0, TH1F* h_3=0x0, TH1F* h_4=0x0)
{
    // go into batch mode where canvases are not drawn:
    gROOT->SetBatch(kTRUE);

    TCanvas* canv = new TCanvas("InvMass","",1200,820);

    Double_t integral = hist1->Integral(1,hist1->GetSize()-2); 
    Double_t integral_2(-1), integral_3(-1), integral_4(-1);
    if (h_2) integral_2 = h_2->Integral(1,h_2->GetSize()-2); 
    if (h_3) integral_3 = h_3->Integral(1,h_3->GetSize()-2); 
    if (h_4) integral_4 = h_4->Integral(1,h_4->GetSize()-2); 

    TString n_1, n_2, n_3, n_4;
    n_1.Form("%i", Int_t(integral));
    n_2.Form("%3.2f", 100.*Double_t(integral_2)/Double_t(integral));
    n_3.Form("%3.2f", 100.*Double_t(integral_3)/Double_t(integral));
    n_4.Form("%3.2f", 100.*Double_t(integral_4)/Double_t(integral));

    hist1->Sumw2();
    if (h_2) h_2->Sumw2();
    if (h_3) h_3->Sumw2();
    if (h_4) h_4->Sumw2();

    // histogram specific actions
    hist1->SetLineColor(kBlack);
    hist1->SetLineWidth(2);
    hist1->SetMarkerStyle(20);
    hist1->SetMarkerSize(1.4);
    hist1->SetMarkerColor(kBlack);

    hist1->GetXaxis()->SetTitle("m_{#pi#pi} (GeV/c^{2})");
    hist1->GetYaxis()->SetTitle("Counts / (0.03 GeV/c^{2})");
    /* axis labels have to be set seperately, */
    /* gStyle is somehow set to default for the next adjustments */
    hist1->GetXaxis()->SetTitleOffset(1.1);
    hist1->GetYaxis()->SetTitleOffset(1.15);
    hist1->GetYaxis()->SetTitleSize(0.055);
    hist1->GetXaxis()->SetTitleSize(0.055);
    hist1->GetXaxis()->SetTitleFont(42);
    hist1->GetYaxis()->SetTitleFont(42);
    hist1->GetXaxis()->SetLabelSize(0.05);
    hist1->GetYaxis()->SetLabelSize(0.05);
    hist1->GetXaxis()->SetLabelFont(42);
    hist1->GetYaxis()->SetLabelFont(42);

    hist1->SetAxisRange(0., 2.5,"X");

    hist1->Draw();

    if (h_2) {
        // 6 = magenta
        h_2->SetLineColor(6);
        h_2->SetLineWidth(2);
        h_2->SetMarkerStyle(24);
        h_2->SetMarkerSize(1.4);
        h_2->SetMarkerColor(6);
        h_2->Draw("same");
    }
    if (h_3) {
        // 8 = dark green
        h_3->SetLineColor(8);
        h_3->SetLineWidth(2);
        // 21 = filled square
        h_3->SetMarkerStyle(21);
        h_3->SetMarkerSize(1.4);
        h_3->SetMarkerColor(8);
        h_3->Draw("same");
    }
    if (h_4) {
        h_4->SetLineColor(kBlue);
        h_4->SetLineWidth(2);
        h_4->SetMarkerStyle(25);
        h_4->SetMarkerSize(1.4);
        h_4->SetMarkerColor(kBlue);
        h_4->Draw("same");
    }

    TLatex tex;
    tex.DrawLatex(1.11086, 622.392, "#splitline{#splitline{ALICE simulation, this work}{Pythia-8 MBR (#varepsilon=0.08)}}{#sqrt{s}=13 TeV}");

    TLegend* leg = new TLegend(0.548495, 0.385732, 0.881271, 0.614268);
    leg->AddEntry(hist1, (Title(hist1)).Data(), "pe");
    if (h_2) leg->AddEntry(h_2, (Title(h_2) + "(" + n_2 + "%)").Data(), "pe");
    if (h_3) leg->AddEntry(h_3, (Title(h_3) + "(" + n_3 + "%)").Data(), "pe");
    if (h_4) leg->AddEntry(h_4, (Title(h_4) + "(" + n_4 + "%)").Data(), "pe");
    /* leg->SetTextSize(0.04); */
    leg->SetFillStyle(0);
    leg->Draw();

    canv->Update();
    canv->SaveAs(output_file.Data());

    printf("Saving add-output in %s\n", output_file.Data());

    /* delete canv; */
}

TString Title(TH1F* hist)
{
    TString out_str;
    TString title_str = hist->GetTitle();
    if (title_str=="fInvMass_FD") out_str = "Feed down";
    else if (title_str=="fInvMass_FD_emcal") out_str = "only #gamma";
    else if (title_str=="fInvMass_FD_3plus") out_str = "3+ tracks";
    else if (title_str=="fInvMass_FD_other") out_str = "other";
    else if (title_str=="fInvMass_FD_hasGammas") out_str = "FD has #gamma";
    else if (title_str=="fInvMass_3trks") out_str = "3 track BG";
    else if (title_str=="fInvMass_4trks") out_str = "4 track BG";
    else if (title_str=="fInvMass_5trks") out_str = "5 track BG";
    else if (title_str=="fInvMass_6trks") out_str = "6 track BG";
    else if (title_str=="fInvMass_3plusTrks") out_str = "3-10 track BG";
    else if (title_str=="fInvMass_GammaDet_sig") out_str = "#gamma hit BG (sig)";
    else if (title_str=="fInvMass_GammaDet_bg")  out_str = "#gamma hit BG (FD)";
    else if (title_str=="fInvMass_LS_plus")  out_str = "Like sign (+)";
    else if (title_str=="fInvMass_LS_minus") out_str = "Like sign (-)";
    else if (title_str=="fNb_trks_passed") out_str = "N_{tracks}";
    else out_str = hist->GetTitle();
    
    return out_str;
}
