/// ~~~{.C}

#include <iostream>
#include <stdlib.h> 
#include <sys/stat.h>

#include <TROOT.h>
#include <TSystem.h>
#include <TDirectory.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <TString.h>
#include <TFile.h>
#include <TChain.h>
#include <TTree.h>

void GetRawEventBuffer(TString input_dirname, CEPRawEventBuffer*& cep_raw_evt, CEPEventBuffer*& cep_evt)
{
    // file extensions
    const char *file_ext = ".root";
    
    // tree and branch name
    TString tree_name = "CEP";
    TString cep_evts_branchname = "CEPEvents";
    TString cep_raw_evts_branchname = "CEPRawEvents";

    TSystemDirectory input_dir(input_dirname, input_dirname);
    TList *input_files = input_dir.GetListOfFiles();

    TChain *cep_tree = new TChain(tree_name);
    if(input_files && (!input_dirname.EndsWith(".root")) ) {
        std::cout << "\nReading \"*" << file_ext << "\" files from \"" << input_dirname << "\"..." << std::endl;
        TSystemFile *file;
        TString fname;
        TIter next(input_files);
        while((file = (TSystemFile*)next())) {
            fname = file->GetName();
            if(!file->IsDirectory() && fname.EndsWith(file_ext)) {
                cep_tree->Add(input_dirname + fname);
            }
        }
    } else if(input_dirname.EndsWith(".root")) cep_tree->Add(input_dirname);
    else { printf("<E> Input file must end with .txt"); gSystem->Exit(0); } 

    cep_raw_evt = 0x0;
    cep_evt = 0x0;

    cep_tree->SetBranchAddress(cep_evts_branchname,     &cep_evt);
    cep_tree->SetBranchAddress(cep_raw_evts_branchname, &cep_raw_evt);
}

