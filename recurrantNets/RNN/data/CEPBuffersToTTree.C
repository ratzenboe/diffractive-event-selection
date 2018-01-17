/// ~~~{.C}

#include <TTree.h>
#include <TFile.h>
#include <TString.h>
#include <iostream>


void FillOutTrees(const char* filename)
{
    TFile* CEPfile = 0x0;
    try {
        CEPfile = TFile::Open(filename);
        if (!CEPfile) throw "File does not exist!\n";
    } catch (const char* errmsg){
        std::cerr << errmsg << std::endl; 
    }
    TTree* CEPtree = (TTree*)CEPfile->Get("CEP");

    CEPRawEventBuffer* CEPRawEvts = 0x0;
    CEPEventBuffer* CEPEvts = 0x0;

    TString save_dir("/media/hdd/train_files/");

    CEPtree->SetBranchAddress("CEPRawEvents", &CEPRawEvts);
    CEPtree->SetBranchAddress("CEPEvents", &CEPEvts);
    // event number is a global number which is the recall variable for
    // every detector, track etc
    UInt_t event_nb;

    TFile* eventFile = new TFile((save_dir+"event_info.root").Data(), "RECREATE");
    TTree* eventTree = new TTree("event", "event level info");
    Int_t evt_n_tracks_total, mc_process_type;
    Double_t evt_tot_ad_mult, evt_tot_ad_time, evt_tot_ad_charge,
             evt_tot_fmd_mult,
             evt_tot_v0_mult, evt_tot_v0_time, evt_tot_v0_charge, evt_tot_v0_sig_width,
             evt_tot_emc_ampl, evt_tot_emc_time,
             evt_tot_phos_ampl, evt_tot_phos_time;
    eventTree->Branch("event_id", &event_nb);
    eventTree->Branch("n_tracks_total", &evt_n_tracks_total);
    eventTree->Branch("tot_ad_mult", &evt_tot_ad_mult);
    eventTree->Branch("tot_ad_time", &evt_tot_ad_time);
    eventTree->Branch("tot_ad_charge", &evt_tot_ad_charge);
    eventTree->Branch("tot_fmd_mult", &evt_tot_fmd_mult);
    eventTree->Branch("tot_v0_mult", &evt_tot_v0_mult);
    eventTree->Branch("tot_v0_charge", &evt_tot_v0_charge);
    eventTree->Branch("tot_v0_time", &evt_tot_v0_time);
    eventTree->Branch("tot_v0_sig_width", &evt_tot_v0_sig_width);
    eventTree->Branch("tot_emc_ampl", &evt_tot_emc_ampl);
    eventTree->Branch("tot_emc_time", &evt_tot_emc_time);
    eventTree->Branch("tot_phos_ampl", &evt_tot_phos_ampl);
    eventTree->Branch("tot_phos_time", &evt_tot_phos_time);

    TFile* trackFile = new TFile((save_dir+"track_info.root").Data(), "RECREATE");
    TTree* trackTree = new TTree("track", "high level track info");
    // hlt = high level track
    Double_t hlt_tof_bunch_crossing, hlt_dca_vtx_z,
             hlt_px, hlt_py, hlt_pz, hlt_pid_global, 
             // TPC
             hlt_pid_tpc_status, hlt_pid_tpc_signal, 
             hlt_pid_tpc_n_sigma_pion, hlt_pid_tpc_n_sigma_kaon, hlt_pid_tpc_n_sigma_proton,
             hlt_pid_tpc_proba_pion, hlt_pid_tpc_proba_kaon, hlt_pid_tpc_proba_proton,
             // TOF
             hlt_pid_tof_status, hlt_pid_tof_signal, 
             hlt_pid_tof_n_sigma_pion, hlt_pid_tof_n_sigma_kaon, hlt_pid_tof_n_sigma_proton,
             hlt_pid_tof_proba_pion, hlt_pid_tof_proba_kaon, hlt_pid_tof_proba_proton,
             // Bayes 
             hlt_pid_bayes_status, 
             hlt_pid_bayes_proba_pion, hlt_pid_bayes_proba_kaon, hlt_pid_bayes_proba_proton;
    trackTree->Branch("event_id", &event_nb);
    trackTree->Branch("tof_bunch_crossing", &hlt_tof_bunch_crossing);
    trackTree->Branch("dca_vtx_z", &hlt_dca_vtx_z);
    trackTree->Branch("px", &hlt_px);
    trackTree->Branch("py", &hlt_py);
    trackTree->Branch("pz", &hlt_pz);
    trackTree->Branch("pid_global", &hlt_pid_global);
    trackTree->Branch("pid_tpc_status", &hlt_pid_tpc_status);
    trackTree->Branch("pid_tpc_signal", &hlt_pid_tpc_signal);
    trackTree->Branch("pid_tpc_n_sigma_pion", &hlt_pid_tpc_n_sigma_pion);
    trackTree->Branch("pid_tpc_n_sigma_kaon", &hlt_pid_tpc_n_sigma_kaon);
    trackTree->Branch("pid_tpc_n_sigma_proton", &hlt_pid_tpc_n_sigma_proton);
    trackTree->Branch("pid_tpc_proba_pion", &hlt_pid_tpc_proba_pion);
    trackTree->Branch("pid_tpc_proba_kaon", &hlt_pid_tpc_proba_kaon);
    trackTree->Branch("pid_tpc_proba_proton", &hlt_pid_tpc_proba_proton);
    trackTree->Branch("pid_tof_status", &hlt_pid_tof_status);
    trackTree->Branch("pid_tof_signal", &hlt_pid_tof_signal);
    trackTree->Branch("pid_tof_n_sigma_pion", &hlt_pid_tof_n_sigma_pion);
    trackTree->Branch("pid_tof_n_sigma_kaon", &hlt_pid_tof_n_sigma_kaon);
    trackTree->Branch("pid_tof_n_sigma_proton", &hlt_pid_tof_n_sigma_proton);
    trackTree->Branch("pid_tof_proba_pion", &hlt_pid_tof_proba_pion);
    trackTree->Branch("pid_tof_proba_kaon", &hlt_pid_tof_proba_kaon);
    trackTree->Branch("pid_tof_proba_proton", &hlt_pid_tof_proba_proton);
    trackTree->Branch("pid_bayes_status", &hlt_pid_bayes_status);
    trackTree->Branch("pid_bayes_proba_pion", &hlt_pid_bayes_proba_kaon);
    trackTree->Branch("pid_bayes_proba_kaon", &hlt_pid_bayes_proba_pion);
    trackTree->Branch("pid_bayes_proba_proton", &hlt_pid_bayes_proba_proton);

    TFile* adFile = new TFile((save_dir+"ad_info.root").Data(), "RECREATE");
    TTree* adTree = new TTree("AD", "raw AD info");
    Double_t AD_mult, AD_charge, AD_time;
    adTree->Branch("event_id", &event_nb);
    adTree->Branch("multiplicity", &AD_mult);
    adTree->Branch("adc_charge", &AD_charge);
    adTree->Branch("time", &AD_time);
    
    TFile* fmdFile = new TFile((save_dir+"fmd_info.root").Data(), "RECREATE");
    TTree* fmdTree = new TTree("FMD", "raw FMD info");
    Double_t FMD_mult;
    fmdTree->Branch("event_id", &event_nb);
    fmdTree->Branch("multiplicity", &FMD_mult);

    TFile* v0File = new TFile((save_dir+"v0_info.root").Data(), "RECREATE");
    TTree* v0Tree = new TTree("V0", "raw V0 info");
    Double_t v0_mult, v0_charge, v0_time, v0_sigwidth;
    v0Tree->Branch("event_id", &event_nb);
    v0Tree->Branch("multiplicity", &FMD_mult);
    v0Tree->Branch("adc_charge", &v0_charge);
    v0Tree->Branch("time", &v0_time);
    v0Tree->Branch("signal_width", &v0_sigwidth);

    TFile* rawTrackFile = new TFile((save_dir+"raw_track_info.root").Data(), "RECREATE");
    TTree* rawTrackTree = new TTree("Tracking", "raw tracking info");
    Double_t trackLength, globalChi2, pidITSsig, pidHMPIDsig, pidTRDsig, pidTOFsig_raw;
    Double_t track_xy, track_z, track_dz, track_dx, track_phiEMC, track_etaEMC, track_pEMC, track_ptEMC;
    rawTrackTree->Branch("event_id", &event_nb);
    rawTrackTree->Branch("track_length", &trackLength);
    rawTrackTree->Branch("global_chi2", &globalChi2);
    rawTrackTree->Branch("pid_its_signal", &pidITSsig);
    rawTrackTree->Branch("pid_hmpid_signal", &pidHMPIDsig);
    rawTrackTree->Branch("pid_trd_signal", &pidTRDsig);
    rawTrackTree->Branch("pid_tof_signal_raw", &pidTOFsig_raw);
    rawTrackTree->Branch("xy_impact", &track_xy);
    rawTrackTree->Branch("z_impact", &track_z);
    rawTrackTree->Branch("dx_tof_impact", &track_dx);
    rawTrackTree->Branch("dz_tof_impact", &track_dz);
    rawTrackTree->Branch("phi_on_emc", &track_phiEMC);
    rawTrackTree->Branch("eta_on_emc", &track_etaEMC);
    rawTrackTree->Branch("pt_on_emc", &track_ptEMC);
    rawTrackTree->Branch("p_on_emc", &track_pEMC);

    TFile* caloClusterFile = new TFile((save_dir+"calo_cluster_info.root").Data(), "RECREATE");
    TTree* caloClusterTree = new TTree("CaloCluster", "raw calo cluster info");
    Double_t CC_E, CC_shapeDispersion, CC_chi2, CC_CPVDist;
    caloClusterTree->Branch("event_id", &event_nb);
    caloClusterTree->Branch("energy", &CC_E);
    caloClusterTree->Branch("shape_dispersion", &CC_shapeDispersion);
    caloClusterTree->Branch("chi2", &CC_chi2);
    caloClusterTree->Branch("cpv_distance", &CC_CPVDist);

    TFile* emcalFile = new TFile((save_dir+"emcal_info.root").Data(), "RECREATE");
    TTree* emcalTree = new TTree("EMCAL", "raw emcal info");
    Double_t emcal_amplidude, emcal_time; 
    emcalTree->Branch("event_id", &event_nb);
    emcalTree->Branch("amplitude", &emcal_amplidude);
    emcalTree->Branch("time", &emcal_time);

    TFile* phosFile = new TFile((save_dir+"phos_info.root").Data(), "RECREATE");
    TTree* phosTree = new TTree("PHOS", "raw phos info");
    Double_t phos_amplidude, phos_time; 
    phosTree->Branch("event_id", &event_nb);
    phosTree->Branch("amplitude", &emcal_amplidude);
    phosTree->Branch("time", &emcal_time);

    // now that we have prepared all Trees and files we go along and read out the 
    // raw-buffers and write the content to the output files
    CEPRawADBuffer* ad      = 0x0;
    CEPRawFMDBuffer* fmd    = 0x0;
    CEPRawV0Buffer* v0      = 0x0;
    CEPRawCaloBuffer* emcal = 0x0;
    CEPRawCaloBuffer* phos  = 0x0;
    
    CEPEventBuffer* event = 0x0;

    for (UInt_t ii(0); ii<CEPtree->GetEntries(); ii++){
        CEPtree->GetEntry(ii);
        if (ii==0){
            std::cout << "Nb tracks in CEPEvts: " << CEPEvts->GetnTracks() << std::endl;
            std::cout << "Nb tracks in CEPRawEvts: " << CEPRawEvts->GetnTracksTotal() << std::endl;
        } else if (ii%1000 == 0) std::cout << ii << " events read" << std::endl;
        event_nb = ii;
        
        // Event  
        evt_n_tracks_total = CEPRawEvts->GetnTracksTotal();

        evt_tot_ad_mult = CEPRawEvts->GetTotalADMult();
        evt_tot_ad_time = CEPRawEvts->GetTotalADTime();
        evt_tot_ad_charge = CEPRawEvts->GetTotalADCharge();

        evt_tot_fmd_mult = CEPRawEvts->GetTotalFMDMult();

        evt_tot_v0_mult = CEPRawEvts->GetTotalV0Mult();
        evt_tot_v0_time = CEPRawEvts->GetTotalV0Time();
        evt_tot_v0_charge = CEPRawEvts->GetTotalV0Charge();
        evt_tot_v0_sig_width = CEPRawEvts->GetTotalV0SigWidth();
       
        evt_tot_emc_ampl = CEPRawEvts->GetTotalEMCAmplitude();
        evt_tot_emc_time = CEPRawEvts->GetTotalEMCTime();
        evt_tot_phos_ampl = CEPRawEvts->GetTotalPHOSAmplitude();
        evt_tot_phos_time = CEPRawEvts->GetTotalPHOSTime();

        eventTree->Fill();

        // HL track info
        CEPTrackBuffer* trk = 0x0;
        TVector3 v;
        for (UInt_t kk(0); kk<CEPEvts->GetnTracks(); kk++){
            trk = CEPEvts->GetTrack(kk);
            if (!trk) break;
            // put here all track info
            hlt_tof_bunch_crossing = trk->GetTOFBunchCrossing(); 
            hlt_dca_vtx_z = trk->GetZv(); 
            // momentum 
            v = trk->GetMomentum(); 
            hlt_px = v.Px();
            hlt_py = v.Py();
            hlt_pz = v.Pz();
            // PID
            hlt_pid_global = trk->GetPID();
            // TPC
            hlt_pid_tpc_status = trk->GetPIDTPCStatus();
            hlt_pid_tpc_signal = trk->GetPIDTPCSignal();
            // n sigma
            hlt_pid_tpc_n_sigma_pion = trk->GetPIDTPCnSigma(AliPID::kPion);
            hlt_pid_tpc_n_sigma_kaon = trk->GetPIDTPCnSigma(AliPID::kKaon);
            hlt_pid_tpc_n_sigma_proton = trk->GetPIDTPCnSigma(AliPID::kProton);
            // probability
            hlt_pid_tpc_proba_pion = trk->GetPIDTPCProbability(AliPID::kPion);
            hlt_pid_tpc_proba_kaon = trk->GetPIDTPCProbability(AliPID::kKaon);
            hlt_pid_tpc_proba_proton = trk->GetPIDTPCProbability(AliPID::kProton);
            // TOF 
            hlt_pid_tof_status = trk->GetPIDTOFStatus();
            hlt_pid_tof_signal = trk->GetPIDTOFSignal();
            // n sigma
            hlt_pid_tof_n_sigma_pion = trk->GetPIDTOFnSigma(AliPID::kPion);
            hlt_pid_tof_n_sigma_kaon = trk->GetPIDTOFnSigma(AliPID::kKaon);
            hlt_pid_tof_n_sigma_proton = trk->GetPIDTOFnSigma(AliPID::kProton);
            // probability
            hlt_pid_tof_proba_pion = trk->GetPIDTOFProbability(AliPID::kPion);
            hlt_pid_tof_proba_kaon = trk->GetPIDTOFProbability(AliPID::kKaon);
            hlt_pid_tof_proba_proton = trk->GetPIDTOFProbability(AliPID::kProton);
            // Bayes
            hlt_pid_bayes_status = trk->GetPIDBayesStatus();
            hlt_pid_bayes_proba_pion = trk->GetPIDBayesProbability(AliPID::kPion);
            hlt_pid_bayes_proba_kaon = trk->GetPIDBayesProbability(AliPID::kKaon);
            hlt_pid_bayes_proba_proton = trk->GetPIDBayesProbability(AliPID::kProton);

            trackTree->Fill();
        }
        
        // AD
        ad = CEPRawEvts->GetRawADBuffer();
        for (UInt_t kk(0); kk<ad->GetNCells(); kk++){
            AD_mult = ad->GetADMultiplicity(kk); 
            AD_time = ad->GetADTime(kk);
            AD_charge = ad->GetADCharge(kk);

            adTree->Fill(); 
        }
        // FMD
        fmd = CEPRawEvts->GetRawFMDBuffer();
        for (UInt_t kk(0); kk<fmd->GetFMDnCells(); kk++){
            FMD_mult = fmd->GetFMDCellMultiplicity(kk);

            fmdTree->Fill();
        }
        // V0 
        v0 = CEPRawEvts->GetRawV0Buffer();
        for (UInt_t kk(0); kk<v0->GetNCells(); kk++){
            v0_mult = v0->GetV0Multiplicity(kk);
            v0_time = v0->GetV0Time(kk);
            v0_charge = v0->GetV0Charge(kk);
            v0_sigwidth = v0->GetV0Width(kk);

            v0Tree->Fill();
        }
        // EMCAL
        emcal = CEPRawEvts->GetRawEMCalBuffer();
        for (UInt_t kk(0); kk<emcal->GetNCells(); kk++){
            emcal_amplidude = emcal->GetCaloCellAmplitude(kk);
            emcal_time =  emcal->GetCaloCellTime(kk);

            emcalTree->Fill();
        }
        // PHOS
        phos = CEPRawEvts->GetRawPHOSBuffer();
        for (UInt_t kk(0); kk<phos->GetNCells(); kk++){
            phos_amplidude = phos->GetCaloCellAmplitude(kk);
            phos_amplidude =  phos->GetCaloCellTime(kk);

            phosTree->Fill();
        }
        // Tracks 
        CEPRawTrackBuffer* rawTrack = 0x0;
        for (UInt_t kk(0); kk<CEPRawEvts->GetnTracksTotal(); kk++){
            rawTrack = CEPRawEvts->GetTrack(kk);
            if (!rawTrack) break;
            // put here all track info
            trackLength = rawTrack->GetTrackLength();
            globalChi2 = rawTrack->GetGlobalChi2();

            pidITSsig = rawTrack->GetPIDITSsig();
            pidHMPIDsig = rawTrack->GetPIDHMPsig();
            pidTRDsig = rawTrack->GetPIDTRDsig();
            pidTOFsig_raw = rawTrack->GetPIDTOFsigRaw();

            track_xy = rawTrack->GetImpactXY();
            track_z = rawTrack->GetImpactZ();
            track_dx = rawTrack->GetTOFImpactDx();
            track_dz = rawTrack->GetTOFImpactDz();

            track_phiEMC = rawTrack->GetTrkPhiOnEMC();
            track_etaEMC = rawTrack->GetTrkEtaOnEMC();
            track_ptEMC = rawTrack->GetTrkPtOnEMC();
            track_pEMC = rawTrack->GetTrkPOnEMC();

            rawTrackTree->Fill();
        }
        // Calo Clusters 
        CEPRawCaloClusterTrack* rawCaloCluster = 0x0;
        for (UInt_t kk(0); kk<CEPRawEvts->GetnCaloClusterTotal(); kk++){
            rawCaloCluster = CEPRawEvts->GetCaloClusterTrack(kk);
            if (!rawCaloCluster) break;
            // put here all track info
            CC_E = rawCaloCluster->GetCaloClusterE();
            CC_shapeDispersion = rawCaloCluster->GetCaloClusterShapeDispersion();
            CC_chi2 = rawCaloCluster->GetCaloClusterChi2();
            CC_CPVDist = rawCaloCluster->GetCaloClusterCPVDist();

            caloClusterTree->Fill();
        }

    }
    rawTrackFile->cd();
    rawTrackTree->Write();
    rawTrackFile->Close();
    delete rawTrackFile;

    phosFile->cd();
    phosTree->Write();
    phosFile->Close();
    delete phosFile;

    emcalFile->cd();
    emcalTree->Write();
    emcalFile->Close();
    delete emcalFile;

    caloClusterFile->cd();
    caloClusterTree->Write();
    caloClusterFile->Close();
    delete caloClusterFile;

    eventFile->cd();
    eventTree->Write();
    eventFile->Close();
    delete eventFile;

    trackFile->cd();
    trackTree->Write();
    trackFile->Close();
    delete trackFile;

    adFile->cd();
    adTree->Write();
    adFile->Close();
    delete adFile;

    fmdFile->cd();
    fmdTree->Write();
    fmdFile->Close();
    delete fmdFile;

    v0File->cd();
    v0Tree->Write();
    v0File->Close();
    delete v0File;

}
