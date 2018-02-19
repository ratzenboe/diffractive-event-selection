/// ~~~{.C}

#include <TTree.h>
#include <TFile.h>
#include <TString.h>
#include <iostream>
#include <stdlib.h> 


void CEPBuffersToTTree(const char* filename, Int_t file_addon = -1)
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
    TString file_addon_str(TString::Itoa(file_addon, 10));
    if (file_addon == -1) file_addon_str = ".root";
    else {
        file_addon_str.Insert(0, "_");
        file_addon_str += ".root";
    }

    CEPtree->SetBranchAddress("CEPRawEvents", &CEPRawEvts);
    CEPtree->SetBranchAddress("CEPEvents", &CEPEvts);
    // event number is a global number which is the recall variable for
    // every detector, track etc
    UInt_t event_nb;

    TFile* eventFile = new TFile((save_dir+"event_info"+file_addon_str).Data(), "RECREATE");
    TTree* eventTree = new TTree("event", "event level info");
    Int_t evt_n_tracks, evt_n_tracklets, evt_n_singles, evt_n_residuals, 
          evt_n_tracks_total, evt_n_tracks_its_only,
          mc_process_type, evt_is_full_recon,
          evt_n_v0s, evt_charge_sum;
    Double_t evt_tot_ad_mult, evt_tot_ad_time, evt_tot_ad_charge,
             evt_tot_fmd_mult,
             evt_tot_v0_mult, evt_tot_v0_time, evt_tot_v0_charge, evt_tot_v0_sig_width,
             evt_tot_emc_ampl, evt_tot_emc_time,
             evt_tot_phos_ampl, evt_tot_phos_time;
    eventTree->Branch("event_id", &event_nb);
    // target
    eventTree->Branch("mc_process_type", &mc_process_type);
    eventTree->Branch("is_full_recon", &evt_is_full_recon);
    // number of various tracks
    eventTree->Branch("n_tracks", &evt_n_tracks);
    eventTree->Branch("n_tracklets", &evt_n_tracklets);
    eventTree->Branch("n_singles", &evt_n_singles);
    eventTree->Branch("n_tracks_total", &evt_n_tracks_total);
    eventTree->Branch("n_residuals", &evt_n_residuals);
    eventTree->Branch("n_tracks_its_only", &evt_n_tracks_its_only);
    // detector sums
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
    // number of v0s
    eventTree->Branch("n_v0s", &evt_n_v0s);
    // charge sum, e.g. 2 pi+ = 2, for 2 pi- = -2 and for pi+pi- = 0
    eventTree->Branch("charge_sum", &evt_charge_sum);

    TFile* trackFile = new TFile((save_dir+"track_info"+file_addon_str).Data(), "RECREATE");
    TTree* trackTree = new TTree("track", "high level track info");
    // hlt = high level track
    Double_t hlt_tof_bunch_crossing, hlt_dca_vtx_z,
             hlt_px, hlt_py, hlt_pz, hlt_pid_global, hlt_eta, hlt_phi, hlt_pt, hlt_P,
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
    // charge sign
    Int_t hlt_charge_sign, 
          hlt_n_its_cls, hlt_n_tpc_cls, hlt_n_trd_cls, hlt_n_tpc_shared_cls;
    // low level track features
    Double_t trackLength, globalChi2, pid_its_sig_tuned, pid_its_sig, pid_tpc_sig_tuned, 
             pidHMPIDsig, pidTRDsig, pid_tof_sig_tuned, 
             rawtrk_its_chi2, rawtrk_tpc_chi2;
    Double_t track_xy, track_z, track_dz, track_dx, track_phiEMC, track_etaEMC, 
             track_pEMC, track_ptEMC, rawtrk_eta;
    trackTree->Branch("event_id", &event_nb);
    trackTree->Branch("tof_bunch_crossing", &hlt_tof_bunch_crossing);

    trackTree->Branch("dca_vtx_z", &hlt_dca_vtx_z);
    trackTree->Branch("pt", &hlt_pt);
    trackTree->Branch("eta", &hlt_eta);
    trackTree->Branch("phi", &hlt_phi);
    trackTree->Branch("P", &hlt_P);
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
    // new addons
    trackTree->Branch("charge_sign", &hlt_charge_sign);            
    trackTree->Branch("n_cls_its", &hlt_n_its_cls);
    trackTree->Branch("n_cls_tpc", &hlt_n_tpc_cls);
    trackTree->Branch("n_cls_trd", &hlt_n_trd_cls);
    trackTree->Branch("n_shared_cls_tpc", &hlt_n_tpc_shared_cls);

    // raw/low level track information
    trackTree->Branch("track_length", &trackLength);
    trackTree->Branch("global_chi2", &globalChi2);
    trackTree->Branch("its_chi2", &rawtrk_its_chi2);
    trackTree->Branch("tpc_chi2", &rawtrk_tpc_chi2);
    trackTree->Branch("pid_its_signal_tuned", &pid_its_sig_tuned);
    trackTree->Branch("pid_its_signal", &pid_its_sig);
    trackTree->Branch("pid_hmpid_signal", &pidHMPIDsig);
    trackTree->Branch("pid_trd_signal", &pidTRDsig);
    trackTree->Branch("pid_tpc_signal_tuned", &pid_tpc_sig_tuned);
    trackTree->Branch("pid_tof_signal_tuned", &pid_tof_sig_tuned);
    trackTree->Branch("xy_impact", &track_xy);
    trackTree->Branch("z_impact", &track_z);
    trackTree->Branch("dx_tof_impact", &track_dx);
    trackTree->Branch("dz_tof_impact", &track_dz);
    trackTree->Branch("phi_on_emc", &track_phiEMC);
    trackTree->Branch("eta_on_emc", &track_etaEMC);
    trackTree->Branch("pt_on_emc", &track_ptEMC);
    trackTree->Branch("p_on_emc", &track_pEMC);
    trackTree->Branch("__test_eta", &rawtrk_eta);



    TFile* adFile = new TFile((save_dir+"ad_info"+file_addon_str).Data(), "RECREATE");
    TTree* adTree = new TTree("ad", "raw AD info");
    Double_t AD_mult, AD_charge, AD_time;
    adTree->Branch("event_id", &event_nb);
    adTree->Branch("multiplicity", &AD_mult);
    adTree->Branch("adc_charge", &AD_charge);
    adTree->Branch("time", &AD_time);
    
    TFile* fmdFile = new TFile((save_dir+"fmd_info"+file_addon_str).Data(), "RECREATE");
    TTree* fmdTree = new TTree("fmd", "raw FMD info");
    Double_t FMD_mult;
    fmdTree->Branch("event_id", &event_nb);
    fmdTree->Branch("multiplicity", &FMD_mult);

    TFile* v0File = new TFile((save_dir+"v0_info"+file_addon_str).Data(), "RECREATE");
    TTree* v0Tree = new TTree("v0", "raw V0 info");
    Double_t v0_mult, v0_charge, v0_time, v0_sigwidth;
    v0Tree->Branch("event_id", &event_nb);
    v0Tree->Branch("multiplicity", &FMD_mult);
    v0Tree->Branch("adc_charge", &v0_charge);
    v0Tree->Branch("time", &v0_time);
    v0Tree->Branch("signal_width", &v0_sigwidth);

    TFile* caloClusterFile = new TFile((save_dir+"calo_cluster_info"+file_addon_str).Data(), "RECREATE");
    TTree* caloClusterTree = new TTree("calo_cluster", "raw calo cluster info");
    Double_t CC_E, CC_shapeDispersion, CC_chi2, CC_CPVDist;
    caloClusterTree->Branch("event_id", &event_nb);
    caloClusterTree->Branch("energy", &CC_E);
    caloClusterTree->Branch("shape_dispersion", &CC_shapeDispersion);
    caloClusterTree->Branch("chi2", &CC_chi2);
    caloClusterTree->Branch("cpv_distance", &CC_CPVDist);

    TFile* emcalFile = new TFile((save_dir+"emcal_info"+file_addon_str).Data(), "RECREATE");
    TTree* emcalTree = new TTree("emcal", "raw emcal info");
    Double_t emcal_amplidude, emcal_time; 
    emcalTree->Branch("event_id", &event_nb);
    emcalTree->Branch("amplitude", &emcal_amplidude);
    emcalTree->Branch("time", &emcal_time);

    TFile* phosFile = new TFile((save_dir+"phos_info"+file_addon_str).Data(), "RECREATE");
    TTree* phosTree = new TTree("phos", "raw phos info");
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
    
    std::cout << CEPtree->GetEntries() << " events in the file: " << filename << std::endl;
    for (UInt_t ii(0); ii<CEPtree->GetEntries(); ii++){
        CEPtree->GetEntry(ii);
        if (!CEPEvts) std::cout << "Event number " << ii << " cannot be found!" << std::endl;
        if (ii==0){
            std::cout << "Nb tracks in CEPEvts: " << CEPEvts->GetnTracks() << std::endl;
            std::cout << "Nb tracks in CEPRawEvts: " << CEPRawEvts->GetnTracksTotal() << std::endl;
        } else if (ii%1000 == 0) std::cout << ii << " events read" << std::endl;
        event_nb = ii;
        
        // initialize charge_sum with 0 for every new event
        evt_charge_sum = 0;
        
        // HL track info
        CEPTrackBuffer* trk = 0x0;
        CEPRawTrackBuffer* rawTrack = 0x0;
        TVector3 v;
        UInt_t hlt_kk(0);
        for (UInt_t kk(0); kk<CEPEvts->GetnTracks(); kk++){
            trk = CEPEvts->GetTrack(kk);
            if (!trk) break;
            // put here all track info
            hlt_tof_bunch_crossing = trk->GetTOFBunchCrossing(); 
            hlt_dca_vtx_z = trk->GetZv(); 
            // momentum 
            v = trk->GetMomentum(); 
            hlt_pt = v.Pt();
            hlt_eta = v.Eta();
            hlt_phi = v.Phi();
            hlt_P = v.Mag();
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
            // charge sign
            hlt_charge_sign = trk->GetChargeSign();
            evt_charge_sum += hlt_charge_sign;
            // clusters
            hlt_n_its_cls = trk->GetITSncls();
            hlt_n_tpc_cls = trk->GetTPCncls();
            hlt_n_trd_cls = trk->GetTRDncls();
            hlt_n_tpc_shared_cls = trk->GetTPCnclsS();

            // now we get the low level features from the CEP-raw buffer
            // the hl-track has the information of the index it is stored in
            // we extract that information and use it to get the low-level features
            // of the hl-track stroed in the CEPRawTrack-buffer
            hlt_kk = trk->GetTrackindex();
            rawTrack = CEPRawEvts->GetTrack(hlt_kk);
            if (!rawTrack) break;
            // put here all track info
            trackLength = rawTrack->GetTrackLength();
            globalChi2 = rawTrack->GetGlobalChi2();
            rawtrk_its_chi2 = rawTrack->GetITSChi2();
            rawtrk_tpc_chi2 = rawTrack->GetTPCChi2();

            pid_its_sig_tuned = rawTrack->GetPIDITSsigTunedOnData();
            pid_its_sig = rawTrack->GetPIDITSsig();

            pidHMPIDsig = rawTrack->GetPIDHMPsig();
            pidTRDsig = rawTrack->GetPIDTRDsig();

            pid_tof_sig_tuned = rawTrack->GetPIDTOFsigTunedOnData();
            pid_tpc_sig_tuned = rawTrack->GetPIDTPCsigTunedOnData();

            track_xy = rawTrack->GetImpactXY();
            track_z = rawTrack->GetImpactZ();

            track_dx = rawTrack->GetTOFImpactDx();
            track_dz = rawTrack->GetTOFImpactDz();

            track_phiEMC = rawTrack->GetTrkPhiOnEMC();
            track_etaEMC = rawTrack->GetTrkEtaOnEMC();
            track_ptEMC = rawTrack->GetTrkPtOnEMC();
            track_pEMC = rawTrack->GetTrkPOnEMC();

            rawtrk_eta = rawTrack->GetEta(); 
            trackTree->Fill();
        }
        // Event  
        // event info contains charge sum, hence we have to first loop
        // over the tracks to gain this information
        evt_n_tracks = CEPEvts->GetnTracks();
        evt_n_tracklets = CEPEvts->GetnTracklets(); 
        evt_n_singles = CEPEvts->GetnSingles(); 
        evt_n_residuals = CEPEvts->GetnResiduals();
        evt_n_tracks_total = CEPEvts->GetnTracksTotal(); 
        evt_n_tracks_its_only = CEPEvts->GetnITSpureTracks();

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

        evt_is_full_recon = is_full_recon(CEPEvts);
        mc_process_type = CEPEvts->GetMCProcessType();
        
        evt_n_v0s = CEPEvts->GetnV0();

        eventTree->Fill();

        
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


