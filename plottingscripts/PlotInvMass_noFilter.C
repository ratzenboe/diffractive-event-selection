/// ~~~{.C}

#include <iostream>
#include <string>
#include <stdlib.h> 
#include <sys/stat.h>
#include <time.h>
#include <algorithm>    // std::random_shuffle
#include <vector>   

#include <TROOT.h>
#include <TSystem.h>
#include <TDirectory.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <TList.h>
#include <TString.h>
#include <TFile.h>
#include <TChain.h>
#include <TTree.h>

void PlotInvMass(TString input_dirname)
{
    // first we check if there is a file behind the path_to_evt_id_txt
    // file extensions
    const char *file_ext = ".root";
    
    // tree and branch name
    TString tree_name = "CEP";
    TString cep_evts_branchname = "CEPEvents";

    TSystemDirectory input_dir(input_dirname, input_dirname);
    TList *input_files = input_dir.GetListOfFiles();

    TChain *cep_tree = new TChain(tree_name);
    if(input_files) {
        std::cout << "\nReading \"*" << file_ext << "\" files from \"" << input_dirname << "\"..." << std::endl;
        TSystemFile *file;
        TString fname;
        TIter next(input_files);
        while((file = (TSystemFile*)next())) {
            fname = file->GetName();
            if (fname=="AnalysisResults_DL_258049_00.root") continue;
            if(!file->IsDirectory() && fname.EndsWith(file_ext)) {
                cep_tree->Add(input_dirname + fname);
            }
        }
    }
    else { printf("<E> No files found in %s", input_dirname.Data()); gSystem->Exit(1); }

    CEPEventBuffer* cep_evt = 0x0;
    cep_tree->SetBranchAddress(cep_evts_branchname, &cep_evt);

    TFile* outfile = new TFile("InvarMassOriginal_NoFilter.root", "RECREATE");

    // event level features:
    TH1F* hInvarmass = new TH1F("invar_mass", "", 333, 0, 10.);
    TList* hist_list = new TList();

    // for display purposes only!
    Float_t progress = 0.0;
    Int_t barWidth = 70;
    /////////////////////////////
    for (UInt_t ii(0); ii<cep_tree->GetEntries(); ii++)
    {
        // display purposes only ////////////////////
        /* progress = float(ii)/float(evt_id_vec.size()); */
        progress = float(ii)/float(cep_tree->GetEntries());
        std::cout << "[";
        Int_t pos = barWidth * progress;
        for (Int_t i(0); i < barWidth; ++i) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(progress * 100.0) << " %\r";
        std::cout.flush();
        // display purposes only ////////////////////

        cep_tree->GetEntry(ii);
        if (!cep_evt) {
            std::cout << "Event number " << ii << " cannot be found!" << std::endl;
            gSystem->Exit(1);
        }
        TLorentzVector lor_vec = cep_evt->GetMCParticle();  
        hInvarmass->Fill(lor_vec.M());
    }
    // cursor of status display has to move to the next line
    std::cout << std::endl;
    std::cout << "100%:   conversion finished " << std::endl;
    ///////////////////////
    hist_list->Add(hInvarmass);
 
    outfile->cd();
    hist_list->Write();
    outfile->Close();
    delete outfile;

    std::cout << "\nSaving the TList in InvarMassOriginal_NoFilter.root" << std::endl;
}

