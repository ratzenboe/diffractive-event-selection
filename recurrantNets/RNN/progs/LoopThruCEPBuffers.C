/// ~~~{.C}

#include <iostream>
#include <stdlib.h> 
#include <sys/stat.h>
#include <time.h>

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

void LoopThruCEPBuffers(TString input_dirname)
{
    TFile* file = new TFile("/home/ratzenboe/Documents/ML_master_thesis/ML-repo/recurrantNets/RNN/data/plots/calo_cells_2D.root", "RECREATE");
    TH2F* hist = new TH2F("E vs. pdg", "E vs. pdg", 100, 0., 3., 100., 0., 2500);
    TH1F* hist_ampl = new TH1F("ampl", "ampl", 100, 0.,1.);
    TH1I* hist_pdg = new TH1I("pdg", "pdg", 2500, 0,2500);
    TList* h_list = new TList();
    
    //
    //
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


    CEPRawEventBuffer* cep_raw_evt = 0x0;
    CEPEventBuffer* cep_evt = 0x0;

    cep_tree->SetBranchAddress(cep_evts_branchname,     &cep_evt);
    cep_tree->SetBranchAddress(cep_raw_evts_branchname, &cep_raw_evt);
    
    // for display purposes only!
    Float_t progress = 0.0;
    Int_t barWidth = 70;
    /////////////////////////////
    std::cout << "\nReading events: " << 0 << " - " << cep_tree->GetEntries()-1 << std::endl;

    Int_t cu, mode(0), times(0);
    Bool_t isDG, isNDG;
    Int_t nseltracks;
    for (UInt_t ii(0); ii<cep_tree->GetEntries(); ii++)
    {
        // display purposes only ////////////////////
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
        /* cu: 
         *      ein bitword mit dem man den Filter steuern kann, für Simulationen ist ein
         *      Wert von 74 geeignet.
         *
         * isDG: 
         *      wird dir am Ende sagen, ob das event ev ein CEP Kandidat ist
         *
         * nseltracks:
         *      ist dann die Anzahl tracks im event 
         */ 
        // no AD
        cu = 107;
        nseltracks = LHC16Filter(cep_evt,kFALSE,cu,isDG,isNDG,mode); 
        if (isDG!=kTRUE || nseltracks!=2) continue;

        ////////////////////////////////////////////////////////////////////////////////
        // here we can now put out function
        EventFunction(cep_evt, cep_raw_evt, hist, hist_ampl, hist_pdg);
        ////////////////////////////////////////////////////////////////////////////////
        times++;
    }
    // cursor of status display has to move to the next line
    std::cout << std::endl;
    std::cout << "100%:   conversion finished " << std::endl;
    ///////////////////////
    //
    std::cout << times << " events passed the filter criterium" << std::endl;
    h_list->Add(hist);
    h_list->Add(hist_pdg);
    h_list->Add(hist_ampl);
    file->cd();
    h_list->Write();
    file->Close();
    delete file;
}

Int_t is_full_recon(CEPEventBuffer* cepevt)
{
    if (!cepevt) {
        std::cout << "Event does not exist!" << std::endl;
        exit(EXIT_FAILURE);
    }
    // extract info if event is fully reconstruced
    // by the tracks in the TPC or not 
    TLorentzVector lv_MCparticle;       // initialized by (0., 0., 0., 0.)
    TLorentzVector lv_trks_total;       // initialized by (0., 0., 0., 0.)
    TLorentzVector lv_trk_temp;         // initialized by (0., 0., 0., 0.)
    lv_MCparticle = cepevt->GetMCParticle();
    // looping through the detected tracks
    // to see if they match the 4-mom of the
    // original particle -> if so the whole evt is
    // detected
    CEPTrackBuffer* trk = 0x0;
    for (int kk(0); kk<cepevt->GetnTracks(); kk++) {
        trk = cepevt->GetTrack(kk); 
        if (!trk) break;

        lv_trk_temp.SetVectM(trk->GetMCMomentum(), trk->GetMCMass());
        lv_trks_total += lv_trk_temp;
    }
    if ( abs_val(lv_MCparticle.M() - lv_trks_total.M()) < 10e-3 ) {
        /* std::cout << "CEP Particle: " << std::endl; */
        /* print_lv(lv_MCparticle); */
        /* std::cout << cepevt->GetnTracks() << " Particles: "; */
        /* if (cepevt->GetnTracks() > 2 ) std::cout << "           <------------- MORE THAN 2 TRACKS! "; */
        /* std::cout << "\n"; */
        /* print_lv(lv_trks_total); */
        /* std::cout << "\n--------------------------------------------------------" << std::endl; */
        return 1;
    } else return 0;
}

double abs_val(double val)
{
    if (val > 0.) return val;
    else return -val;
}

void print_lv(TLorentzVector lv)
{
    std::cout << "px: " << lv.Px() << std::endl;
    std::cout << "py: " << lv.Py() << std::endl;
    std::cout << "pz: " << lv.Pz() << std::endl;
    std::cout << "m:  " << lv.M() << std::endl;
}

       
void EventFunction(CEPEventBuffer* cep_evt, CEPRawEventBuffer* cep_raw_evt, TH2F*& hist,
                   TH1F*& hist_ampl, TH1I*& hist_pdg)
{
    CEPRawCaloBuffer* emcal = 0x0;
    CEPRawCaloBuffer* phos = 0x0;
    
    // EMCAL
    Float_t emcal_amplidude, phos_amplidude;
    Int_t emcal_pdg, phos_pdg;
    emcal = cep_raw_evt->GetRawEMCalBuffer();
    for (UInt_t kk(0); kk<emcal->GetNCells(); kk++)
    {
        emcal_amplidude = emcal->GetCaloCellAmplitude(kk);
        emcal_pdg =  emcal->GetCaloCellMCLabel(kk);

        hist_pdg->Fill(emcal_pdg);
        hist_ampl->Fill(emcal_amplidude);
        hist->Fill(emcal_amplidude,emcal_pdg);
    }
    // PHOS
    phos = cep_raw_evt->GetRawPHOSBuffer();
    for (UInt_t kk(0); kk<phos->GetNCells(); kk++)
    {
        phos_amplidude = phos->GetCaloCellAmplitude(kk);
        phos_pdg =  phos->GetCaloCellMCLabel(kk);

        hist_pdg->Fill(phos_pdg);
        hist_ampl->Fill(phos_amplidude);
        hist->Fill(phos_amplidude,phos_pdg);
    }
}

