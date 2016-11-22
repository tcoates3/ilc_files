#include "SignalSeparator.h"
#include <iostream>
#include <math.h>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <EVENT/LCCollection.h>
#include <EVENT/LCRelation.h>
#include <EVENT/MCParticle.h>
#include <EVENT/ReconstructedParticle.h>
#include <UTIL/LCTOOLS.h>
// ----- include for verbosity dependend logging ---------
#include "marlin/VerbosityLevels.h"

#include <marlin/Exceptions.h>

#ifdef MARLIN_USE_AIDA
#include <marlin/AIDAProcessor.h>
#include <AIDA/IHistogramFactory.h>
#include <AIDA/ITree.h>
//#include <AIDA/IHistogram1D.h>
#endif // MARLIN_USE_AIDA


using namespace lcio ;
using namespace marlin ;

bool debug = false;

SignalSeparator aSignalSeparator ;


SignalSeparator::SignalSeparator() : Processor("SignalSeparator") 
{

  // modify processor description
  _description = "";


  // register steering parameters: name, description, class-variable, default value
  registerInputCollection( LCIO::LCRELATION,
			   "InputMCTruthLinkCollection" , 
			   "Name of the RecoMCTruthLink collection"  ,
			   _colMCTL ,
			   std::string("RecoMCTruthLink")
			   );

  registerInputCollection( LCIO::MCPARTICLE,
			   "InputMCParticleCollection" , 
			   "Name of the MCParticle collection"  ,
			   _colMCP ,
			   std::string("MCParticle")
			   //std::string("MCParticlesSkimmed")
			   );

  registerInputCollection( LCIO::RECONSTRUCTEDPARTICLE,
			   "InputJetCollection" , 
			   "Name of the ReconstructedParticle collection"  ,
			   _colJet ,
			   std::string("ReclusteredJets")
			   );
}

void SignalSeparator::init()
{ 

  streamlog_out(DEBUG) << "   init called  " << std::endl ;

  // usually a good idea to
  printParameters() ;

  _nRun = 0 ;
  _nEvt = 0 ;
  _nZ = 0 ;
  _nZQuarks = 0 ;
  _nHiggs = 0 ;
  _nHiggsQuarks = 0 ;
  //nskipped=0;
  //npassed=0;

  //Define persistent variables
  NumberHiggs = 0;
  // Custom variables to keep track of the number of different types of events internally (not ROOT) so they can be printed.
  NumLeptons = 0;
  NumSemileptonic = 0;
  NumHadronic = 0;
  
  


  AIDA::IHistogramFactory* pHistogramFactory=marlin::AIDAProcessor::histogramFactory( this );
  AIDA::ITree* pTree=marlin::AIDAProcessor::tree( this );
  
  if(  pHistogramFactory!=0 )
    {
      if (!(pTree->cd( "/" + name() + "/"))) 
	{
	  pTree->mkdir( "/" + name() + "/" );
	  pTree->cd( "/" + name() + "/");
	}

      // booking all the histograms
      _ptMCPLeptons = new TH1F("ptMCLeptons", "Momenta of MC leptons", 200, 0.0, 100.0);
      _ptRPLeptons = new TH1F("ptRPLeptons", "Momenta of RP leptons", 200, 0.0, 100.0);
      _ptMCPLeptons_vis = new TH1F("ptMCLeptons_vis", "Momenta of MCP leptons (visible)", 200, 0.0, 100.0);
      _ptMCPQuarks = new TH1F("ptMCPQuarks", "Momenta of MCP Quarks", 200, 0.0, 100.0);
      _massMCPZboson = new TH1F("massMCPZboson", "Mass of MCP Z boson", 200, 0.0, 150.0);
      _massMCPHiggs = new TH1F("massMCPHiggs", "Mass of MCP Higgs", 200, 0.0, 150.0);
      _massMCPV = new TH1F("massMCPV", "Mass of MCP V particles", 200, 0.0, 150.0);
      _massMCPbottom = new TH1F("massMCPbottom", "Mass of MCP bottom quark", 200, 0.0, 40.0);
      _massRPV = new TH1F("massRPV", "Mass of RP V particles", 200, 0.0, 150.0);
      _ptRPPDG = new TH1F("RPPDG", "PDG of reco particles", 4601, -2300.0, 2300.0);
      _PDGMCP = new TH1F("PDGMCP", "PDG of MC particles", 4601, -2300.0, 2300.0);
      _ptMCPDG = new TH1F("reco_MCPPDG", "PDG of reco MC particles", 4601, -2300.0, 2300.0);
      _ptRPType = new TH1F("RP_Types", "Types of reco particles", 4601, -2300.0, 2300.0);
      _angle = new TH1F("angle", "angular separation of single Reco particle to all MC particles", 100, 0, M_PI);
      _RPlikelyhood = new TH1F("RPLikelyhood", "likelyhood of reco. particles", 1000, 0.0, 1.0);
      _ptRPtl = new TH1F("pt_all_RP", "Momenta of RP", 200, 0.0, 100.0);
      _ptMCPtl = new TH1F("pt_all_RecoMCP", "Momenta of RPreco MC particles", 200, 0.0, 100.0);
      _generated_MCPtl_pt = new TH1F("pt_generated_MCP", "Momenta of generated MC_reco particles", 200, 0.0, 100.0);
      _generated_RPtl_pt = new TH1F("pt_generated_RPtl", "Momenta of generated Reco particles", 200, 0.0, 100.0);
      _bestGuess = new TH1F("BestGuess_PDG", "PDG of best Guess MC particles", 4601, -2300.0, 2300.0);
      _ptBestGuess = new TH1F("BestGuess_lepton_momentum", "momentum of best_guess reco. leptons", 200, 0.0, 100.0);
      _angleBestGuess = new TH1F("BestGuess_angle", "Best Guess angular separation", 10000, 0, 0.0001);
      _xangle = new TH1F("xangle", "angular separation of single Reco particle to x axis", 3156, 0, M_PI);
      _yangle = new TH1F("yangle", "angular separation of single Reco particle to y axis", 3156, 0 ,M_PI);
      _zangle = new TH1F("zangle", "angular separation of single Reco particle to z axis", 3156, 0, M_PI);
      _ptDiffBestGuess = new TH1F("pt_diff_best", "Best Momenta difference between generated Reco particles", 4000, -1.0, 1.0);
      _ptDiffBadGuess = new TH1F("pt_diff_bad", "Bad Momenta difference between generated Reco particles", 4000, -2.0, 2.0);

      // jet distrobution histograms
      _jet_distro = new TH1I("jet_distro", "Number of jets per event", 10, 0, 10);
      _jetPThisto1 = new TH1F("jet_pt_1", "Jet transverse momentum (1)", 400, 0.0, 400.0);
      _jetPThisto2 = new TH1F("jet_pt_2", "Jet transverse momentum (2)", 200, 0.0, 200.0);
      _jetPThisto3 = new TH1F("jet_pt_3", "Jet transverse momentum (3)", 200, 0.0, 200.0);
      _jetMatchedPThisto = new TH1F("jet_matched_pt","Transverse momentum of all jets combinations", 400, 0.0, 400.0);

	
      _jetsbestGuesspt = new TH1F("jets_mcptl_ang","best guess angle of reconstructed quark jets to MC quarks", 2000, 0.0, 1.50);
      _jetMinP = new TH1F("jet_optimized_momentum","Jet momentum optimized for angles below 1 degree", 400, 0.0, 200.0);

      _jetsBestGuessAngleHiggs = new TH1F("Higgs_distance_BG","best guess angular distance of reconstructed quark jets to MC quarks for Higgs boson", 2000, 0.0, 1.50);
      _jetsBestGuessAngleZ = new TH1F("Z_distance_BG","best guess angle of reconstructed quark jets to MC quarks for Z boson", 2000, 0.0, 1.50);

      _jetPairSep = new TH1F("jet_pair_ang_sep","Jet Pair Separation from each other", 800, 0.0, M_PI);
      _jetOptH = new TH1F("jet_optimized_momentum_higgs","Jet momentum optimized for angles below 0.05 and 0.6 degree for Higgs boson", 400, 0.0, 200.0);
      _jetOptZ = new TH1F("jet_optimized_momentum_Z","Jet momentum optimized for angles below 0.05 and 0.6 degree for Z boson", 400, 0.0, 200.0);

      //_jetInvMass = new TH1F("jet_inv_mass","Jet invariant mass trial", 400, 0.0, 400.0);  // Unused
      _jetInvMassZ = new TH1F("jet_inv_mass_Z","Z Jet invariant mass trial", 400, 0.0, 400.0);
      _jetInvMassHiggs = new TH1F("jet_inv_mass_Higgs","Higgs Jet invariant mass trial", 400, 0.0, 400.0);

      _jetsAngSep = new TH1F("jets_match_ang","best guess angle of reconstructed quark jets pairs to MC quarks using mass", 200, 0.0, M_PI);
      _jetsZAngSep1 = new TH1F("jets_Z_ang1","best guess angle of reconstructed quark jets 1 to MC quarks using mass with Z parents", 200, 0.0, M_PI);
      _jetsZAngSep2 = new TH1F("jets_Z_ang2","best guess angle of reconstructed quark jets 2 to MC quarks using mass with Z parents", 200, 0.0, M_PI);
      _jetsHAngSep1 = new TH1F("jets_H_ang1","best guess angle of reconstructed quark jets 1 to MC quarks using mass with Higgs parents", 2000, 0.0, M_PI);
      _jetsHAngSep2 = new TH1F("jets_H_ang2","best guess angle of reconstructed quark jets 2 to MC quarks using mass with Higgs parents", 2000, 0.0, M_PI);
      _jetInvMassZmcptl= new TH1F("adj_jet_inv_mass_Z","Jet invariant mass for associated MC quark particle with parent Z bosons", 400, 0.0, 200.0);
      _jetInvMassHmcptl= new TH1F("adj_jet_inv_mass_H","Jet invariant mass for associated MC quark particle with parent Higgs bosons", 400, 0.0, 200.0);
      _jetInvMassmcptl= new TH1F("adj_jet_inv_mass","Jet invariant mass for associated MC quark particle for Z and Higgs Parent bosons", 400, 0.0, 300.0);
      _leptonInvmass = new TH1F("lep_inv_mass","leptons invariant mass for Z boson", 400, 0.0, 200.0);

    }
}

void SignalSeparator::processRunHeader( LCRunHeader* run) { 

  _nRun++ ;
} 



void SignalSeparator::processEvent( LCEvent * evt ) 
{ 
  _nEvt ++ ;

  std::cout << " =!=  processing event: " << evt->getEventNumber() 
	    << "   in run:  " << _nRun << " =!= " << std::endl ;

  // some old jet geometry checking

  xAxis = {1,0,0};
  yAxis = {0,1,0};
  zAxis = {0,0,1};
  _higgscounter = 0;
  _Zcounter =0;
  int JetMatchArray[8] = {NULL};
 
  NoMatchParticles = 0;
  
  //MCPARTICLE
  LCCollection* colMCP = NULL;
 
  // opening the particle collection that contains the Monte Carlo particles
  try
    {
      colMCP = evt->getCollection( _colMCP );
    }
  catch( lcio::DataNotAvailableException e )
    {
      streamlog_out(WARNING) << _colMCP << " collection not available" << std::endl;
      colMCP = NULL;
    }
    
  //Define non-persistent variables

  if( colMCP != NULL )
    {
      int nElements = colMCP->getNumberOfElements()  ;

      if(debug){
	std::cout << "nElements = " << nElements << std::endl;
      }
      
      //START LOOP OVER PARTICLE
      for(int i=0; i< nElements ; i++)
	{ 

	  MCParticle* mcp = dynamic_cast<MCParticle*>( colMCP->getElementAt( i ) ) ;

	 

	  if( abs(mcp->getPDG())==25)
	    {
	      _higgscounter++;
	      _massMCPHiggs->Fill(abs(mcp->getMass())); // fill MPC Higgs mass histogram
	      //std::cout << "Parent PDG (Higgs): " << ((mcp->getParents()[0]->getPDG())) << std::endl;
	      //std::cout << "Higgs decays into: " << mcp->getDaughters()[0]->getPDG() << " and " << mcp->getDaughters()[1]->getPDG() << std::endl;
	    }
	  if(abs(mcp->getPDG())<2300)
	    {
	      _PDGMCP->Fill(mcp->getPDG()); // fill MPC PDH histogram
	    }
	  else
	    {
	      _PDGMCP->Fill(0);
	    }
	  if( abs(mcp->getPDG())==23)
	    {
	      _Zcounter++;
	      _massMCPZboson->Fill(abs(mcp->getMass())); // fill MPC Z boson mass histogram
	      // std::cout << "Parent PDG (Z boson): " << ((mcp->getParents()[0]->getPDG())) << std::endl;
	    }
	  if( abs(mcp->getPDG())==25 || abs(mcp->getPDG())==23)
	    {
	      _massMCPV->Fill((mcp->getMass())); // fill MPC V particles mass (higgs and Z) histogram
	      //std::cout << "Parent PDG: " << mcp->getParents()[0] << std::endl;
	    }
	  if( mcp->getParents().size()>0 ) //if particle has a parent
	    {
	      // if( abs(mcp->getPDG())==2212)
	      //	{
	      //_massMCPprotons->Fill((mcp->getMass())); // fill MPC protons mass histogram
	      //std::cout << "Proton mass: " << mcp->getMass() << std::endl;
	      //	}
	      if(abs(mcp->getParents()[0]->getPDG())==25) //if parent is a h0
		{
		  if ((abs(mcp->getPDG())==5)) //if particle is bottom
		    {
		      //_ptMCPbottom->Fill(abs(*(mcp->getMomentum()))); //fill MCPbottom Histogram 
		      _massMCPbottom->Fill(((mcp->getMass()))); //fill MCPbottom Histogram with mass of bottom quarks
		    }
		}
	      if(abs(mcp->getParents()[0]->getPDG())==24) //if parent is a W
		{
		  if(mcp->getParents()[0]->getParents().size()>0) //if particle has a grandparent
		    {
		      if(abs(mcp->getParents()[0]->getParents()[0]->getPDG())==6) //if grandparent is a top
			{
			  //if((abs(mcp->getPDG())==11) || (abs(mcp->getPDG())==13) || (abs(mcp->getPDG())==15)) // if particle is a visible lepton (e,mu,tau)
			  // {
			  //  _ptMCPLeptons_vis->Fill(abs(*(mcp->getMomentum())));//fill MCP_vis Histogram 
			  //}
			  if((abs(mcp->getPDG())>10) && (abs(mcp->getPDG())<17))//If particle is a lepton
			    {
			      _ptMCPLeptons->Fill(sqrt(pow(mcp->getMomentum()[0],2)+pow(mcp->getMomentum()[1],2)+pow(mcp->getMomentum()[2],2)));//fill MC histogram
			      NumLeptons++;
			      //std::cout << "abs(momentum) of leptons: " << abs(*mcp->getMomentum()) << std::endl;
			      //std::cout << "Actual absolute value (leptons): " << sqrt(pow(mcp->getMomentum()[0],2)+pow(mcp->getMomentum()[1],2)+pow(mcp->getMomentum()[2],2)) << std::endl;
			    }
			  if((abs(mcp->getPDG())>0) && (abs(mcp->getPDG())<6)) // if particle is a quark but not top
			    {
			      _ptMCPQuarks->Fill(sqrt(pow(mcp->getMomentum()[0],2)+pow(mcp->getMomentum()[1],2)+pow(mcp->getMomentum()[2],2)));//fill MCPQuarks Histogram 
			      //std::cout << "abs(momentum) of quarks: " << abs(*mcp->getMomentum()) << std::endl;
			      //std::cout << "Actual absolute value (quarks): " << sqrt(pow(mcp->getMomentum()[0],2)+pow(mcp->getMomentum()[1],2)+pow(mcp->getMomentum()[2],2)) << std::endl;
			    }
			}
		    }
		}
	    }
	}
      //END LOOP OVER PARTICLE  
    }

  if (_Zcounter > 0)
    {
      std::cout<<"EVENT CONTAINS "<<_Zcounter<<" Z BOSONS"<<std::endl;
    }
  if (_higgscounter > 0)
    {
      std::cout<<"EVENT CONTAINS "<<_higgscounter<<" HIGGS BOSONS"<<std::endl;
    }






  //JETS
  LCCollection* colJet = NULL;
  try
    {
      colJet = evt->getCollection( _colJet );
    }
  catch( lcio::DataNotAvailableException e )
    {
      streamlog_out(WARNING) << _colJet << " collection not available" << std::endl;
      colJet = NULL;
    }

  if( colJet != NULL )
    {
      int nElements = colJet->getNumberOfElements();
      int MCElements = colMCP->getNumberOfElements();
      
      if(debug){
	std::cout << "nElements = " << nElements << std::endl;
      }

      _jet_distro->Fill(nElements);
      jetPT = {0};
      jetPx = {0};
      jetPy = {0};
      jetPz = {0};
      jetE = {0};
      bestGuessInvMassZ = 10000.0;
      bestGuessInvMassHiggs = 10000.0;
      BestGuessAnglemcpt = 100.0;
      jet1IndZ=0.0;
      jet2IndZ=0.0;
      jet1IndHiggs=0.0;
      jet2IndHiggs=0.0;
      //START LOOP OVER JET
      for(int i=0; i< nElements ; i++)
	{
	  // Opening jet objects
	  ReconstructedParticle* thisjet = dynamic_cast<ReconstructedParticle*>(colJet->getElementAt(i));  
	  // Finding transverse momentum of this jet and storing it in our array.
	  //jetPT[i] = sqrt(pow(thisjet->getMomentum()[0],2) + pow(thisjet->getMomentum()[1],2)); //TRANSVERSE MOMENTUM
	  jetPx[i]=thisjet->getMomentum()[0];
	  jetPy[i]=thisjet->getMomentum()[1];
	  jetPz[i]=thisjet->getMomentum()[2];
	  jetPT[i] = sqrt(pow(thisjet->getMomentum()[0],2) + pow(thisjet->getMomentum()[1],2) + pow(thisjet->getMomentum()[2],2)); // MOMENTUM MAGNITUDE
	  jetE[i] = thisjet->getEnergy();
	  //thisjetPMag = sqrt(pow(thisjet->getMomentum()[0],2) + pow(thisjet->getMomentum()[1],2) + pow(thisjet->getMomentum()[2],2));
	} //TEMPORARY FOR LOOP BRAKET//
      
      for(int z=0; z < nElements; z++)
	{
	  for (int l=0; l < nElements;l++)
	    {
	      if(z < l)
		{ 
		  invmass = sqrt(pow(jetE[z]+jetE[l],2)-(pow((jetPx[z]+jetPx[l]),2)+pow((jetPy[z]+jetPy[l]),2)+pow((jetPz[z]+jetPz[l]),2)));
		  _jetMatchedPThisto->Fill(jetPT[z]+jetPT[l]);
		  if (fabs(91.2-invmass)<fabs(91.2-bestGuessInvMassZ))
		    {
		      bestGuessInvMassZ = invmass;
		      jet1IndZ = z;
		      jet2IndZ = l;
		    }  
		  else if (fabs(125.0-invmass)<fabs(125.0-bestGuessInvMassHiggs))
		    {
		      bestGuessInvMassHiggs = invmass;
		      jet1IndHiggs = z;
		      jet2IndHiggs = l;
		    }
		  else
		    {
		      if (debug) {std::cout<<"EXCLUDED JET DATA HERE"<<std::endl;}
		    }
		}
	    }
	}

      if ( fabs(91.2-bestGuessInvMassZ) < fabs(125.0-bestGuessInvMassHiggs) )
   	{
	  _jetInvMassZ->Fill(bestGuessInvMassZ);
	}
      else if( fabs(91.2-bestGuessInvMassZ) > fabs(125.0-bestGuessInvMassHiggs) )
	{
	  _jetInvMassHiggs->Fill(bestGuessInvMassHiggs);
	}
      else
	{
	  if (debug) {std::cout << "WARNING: No Z boson or Higgs found in jet matching." << std::endl;}
	}


      //_jetInvMassZ->Fill(bestGuessInvMassZ);
      //_jetInvMassHiggs->Fill(bestGuessInvMassHiggs);
      
      bestGuessZAngle1 = NULL;
      bestGuessZAngle2 = NULL;
      bestGuessHAngle1 = NULL;
      bestGuessHAngle2 = NULL;
      DotProductmcpt1=0.0;
      DotProductmcpt2=0.0;
      jet1Mag=0.0;
      jet2Mag=0.0;
      angle1=0.0;
      angle2=0.0;
      adjInvMass=0.0;

      for(int f = 0; f < MCElements; f++)
	{
	  MCParticle* mcptl = dynamic_cast<MCParticle*>(colMCP->getElementAt(f));
	  if(abs(mcptl->getPDG())==25){_nHiggs++;}
	  if(abs(mcptl->getPDG())==23){_nZ++;}
	  if ( (abs(mcptl->getPDG())<6 && mcptl->getParents().size() > 0) && mcptl->isCreatedInSimulation() == false)
	    {
	      if (abs((mcptl->getParents())[0]->getPDG())==25){_nHiggsQuarks++;}
	      if (abs((mcptl->getParents())[0]->getPDG())==23){_nZQuarks++;}
	      if (abs((mcptl->getParents())[0]->getPDG())==23)// Z BOSON
		{
		  //std::cout<<"Quark PDG: "<<mcptl->getPDG()<<std::endl;
		  mcptPMag = sqrt(pow(mcptl->getMomentum()[0],2)+pow(mcptl->getMomentum()[1],2)+pow(mcptl->getMomentum()[2],2));
		  ReconstructedParticle* jet1= dynamic_cast<ReconstructedParticle*>(colJet->getElementAt(jet1IndZ));
		  ReconstructedParticle* jet2= dynamic_cast<ReconstructedParticle*>(colJet->getElementAt(jet2IndZ));
		  DotProductmcpt1 = std::inner_product(mcptl->getMomentum(),mcptl->getMomentum()+3,jet1->getMomentum(),0.0);
		  DotProductmcpt2 = std::inner_product(mcptl->getMomentum(),mcptl->getMomentum()+3,jet2->getMomentum(),0.0);
		  jet1Mag = sqrt(pow(jet1->getMomentum()[0],2) + pow(jet1->getMomentum()[1],2) + pow(jet1->getMomentum()[2],2));
		  jet2Mag = sqrt(pow(jet2->getMomentum()[0],2) + pow(jet2->getMomentum()[1],2) + pow(jet2->getMomentum()[2],2));
		  angle1 = acos(DotProductmcpt1/(mcptPMag*jet1Mag));
		  angle2 = acos(DotProductmcpt2/(mcptPMag*jet2Mag));
		  
		  if (angle1<angle2)//NULL is the value set for the original value of best guess, hence if bestGuessAngle==NULL then it hasnt been triggered in the if/else loops
		    {
		      if(bestGuessZAngle1 != NULL) 
			{
			  //std::cout<<"if angle 1 PDG: "<<mcptl->getPDG()<<std::endl;
			  bestGuessZAngle1=angle1;
			  // adjInvMass=bestGuessInvMass; //
			}
		      else
			{
			  //std::cout<<"else angle 1 PDG: "<<mcptl->getPDG()<<std::endl;
			  bestGuessZAngle1=angle1;
			  // adjInvMass=bestGuessInvMass; //
			  if (bestGuessZAngle2 == NULL)
			    {
			      //std::cout<<"else if angle 2 PDG: "<<mcptl->getPDG()<<std::endl;
			      bestGuessZAngle2=angle2;
			      // adjInvMass=bestGuessInvMass;// 
			    }
			}

		      //std::cout<<"angle 1 PDG: "<<mcptl->getPDG()<<std::endl;
		      //bestGuessAngle1=angle1;
		      //adjInvMass=bestGuessInvMass;
		    }
		  if(angle2<angle1)
		    {
		      if(bestGuessZAngle2 != NULL)
			{
			  //std::cout<<"if angle 2 PDG: "<<mcptl->getPDG()<<std::endl;
			  bestGuessZAngle2=angle2;
			  // adjInvMass=bestGuessInvMass; //
			}
		      else
			{ 
			  //std::cout<<"else angle 2 PDG: "<<mcptl->getPDG()<<std::endl;
			  bestGuessZAngle2=angle2;
			  //adjInvMass=bestGuessInvMass; //
			  if (bestGuessZAngle1 == NULL)
			    {
			      //std::cout<<"else if angle 1 PDG: "<<mcptl->getPDG()<<std::endl;
			      bestGuessZAngle1=angle1;
			      //adjInvMass=bestGuessInvMass;// 
			    } 
			}
		    }
		  if (angle1 == angle2)
		    {std::cout<<"ERROR DATA HERE"<<std::endl;}
		}

	      if (abs((mcptl->getParents())[0]->getPDG())==25)// HIGGS 
		{
		  //std::cout<<"Quark PDG: "<<mcptl->getPDG()<<std::endl;
		  mcptPMag = sqrt(pow(mcptl->getMomentum()[0],2)+pow(mcptl->getMomentum()[1],2)+pow(mcptl->getMomentum()[2],2));
		  ReconstructedParticle* jet1= dynamic_cast<ReconstructedParticle*>(colJet->getElementAt(jet1IndHiggs));
		  ReconstructedParticle* jet2= dynamic_cast<ReconstructedParticle*>(colJet->getElementAt(jet2IndHiggs));
		  DotProductmcpt1 = std::inner_product(mcptl->getMomentum(),mcptl->getMomentum()+3,jet1->getMomentum(),0.0);
		  DotProductmcpt2 = std::inner_product(mcptl->getMomentum(),mcptl->getMomentum()+3,jet2->getMomentum(),0.0);
		  jet1Mag = sqrt(pow(jet1->getMomentum()[0],2) + pow(jet1->getMomentum()[1],2) + pow(jet1->getMomentum()[2],2));
		  jet2Mag = sqrt(pow(jet2->getMomentum()[0],2) + pow(jet2->getMomentum()[1],2) + pow(jet2->getMomentum()[2],2));
		  angle1 = acos(DotProductmcpt1/(mcptPMag*jet1Mag));
		  angle2 = acos(DotProductmcpt2/(mcptPMag*jet2Mag));
		  
		  if (angle1<angle2)//NULL is the value set for the original value of best guess, hence if bestGuessAngle==NULL then it hasnt been triggered in the if/else loops
		    {
		      if(bestGuessHAngle1 != NULL) 
			{
			  bestGuessHAngle1=angle1;
			  // adjInvMass=bestGuessInvMass; //
			}
		      else
			{
			  bestGuessHAngle1=angle1;
			  // adjInvMass=bestGuessInvMass; //
			  if (bestGuessAngle2 == NULL)
			    {
			      bestGuessHAngle2=angle2;
			      // adjInvMass=bestGuessInvMass;// 
			    }
			}
		    }
		  if(angle2<angle1)
		    {
		      if(bestGuessHAngle2 != NULL)
			{
			  bestGuessHAngle2=angle2;
			  // adjInvMass=bestGuessInvMass; //
			}
		      else
			{ 
			  bestGuessHAngle2=angle2;
			  //adjInvMass=bestGuessInvMass; //
			  if (bestGuessHAngle1 == NULL)
			    {
			      bestGuessHAngle1=angle1;
			      //adjInvMass=bestGuessInvMass;// 
			    } 
			}
		    }
		  if (angle1 == angle2)
		    {std::cout<<"ERROR DATA HERE"<<std::endl;}
		}
	      
	    }
	}//FOR LOOP CLOSE BRAKET

      adjInvMassZ=bestGuessInvMassZ;
      adjInvMassHiggs=bestGuessInvMassHiggs;

      if (bestGuessZAngle1 != NULL || bestGuessZAngle2 != NULL)
	{
	  _jetsAngSep->Fill(bestGuessZAngle1);
	  _jetsAngSep->Fill(bestGuessZAngle2);
	  _jetsZAngSep1->Fill(bestGuessZAngle1);
	  _jetsZAngSep2->Fill(bestGuessZAngle2);
	  
	  // if ((bestGuessAngle1<0.04 || bestGuessAngle2<0.04)) // ANGULAR SEPARATION CUT MAY BE APPLIED HERE
	  {
	    _jetInvMassZmcptl->Fill(adjInvMassZ);
	    _jetInvMassmcptl->Fill(adjInvMassZ);
	  }
	}
      else
	{
	  std::cout << "One of the Z jet angles was NULL!!" << std::endl;
	}
    
      if (bestGuessHAngle1 != NULL || bestGuessHAngle2 != NULL)
	{
	  _jetsAngSep->Fill(bestGuessHAngle1);
	  _jetsAngSep->Fill(bestGuessHAngle2);
	  _jetsHAngSep1->Fill(bestGuessHAngle1);
	  _jetsHAngSep2->Fill(bestGuessHAngle2);
	  
	  // if ((bestGuessAngle1<0.04 || bestGuessAngle2<0.04)) //ANGULAR SEPARATION CUT MAY BE APPLIED HERE
	  {
	    _jetInvMassHmcptl->Fill(adjInvMassHiggs);
	    _jetInvMassmcptl->Fill(adjInvMassHiggs);
	  }
	}
      else
	{
	  std::cout << "One of the Higgs jet angles was NULL!!" << std::endl;
	}

      // MarlinFastJet arranges the jet objects in descending order of momentum, so no sorting is needed/done for the following histograms.

      _jetPThisto1->Fill(jetPT[0]+jetPT[1]); // This is probably the two b quarks from the top decays, which has seems to have no particular meaning.
      if(nElements > 3) {_jetPThisto2->Fill(jetPT[2]+jetPT[3]);}  // This seems to be the b bbar one and thus gives us the V invariant mass.
      if(nElements > 5) {_jetPThisto3->Fill(jetPT[4]+jetPT[5]);}  // So few events in our sample have more than 4 jets that it's difficult to say what this represents, if anything.
    }





  //MCPARTICLES and JETS combination
  LCCollection* colMCPT = NULL;

  try
    {
      colMCPT = evt->getCollection( _colMCP );  
    }
    
  catch( lcio::DataNotAvailableException e )
    {
      streamlog_out(WARNING) << _colMCP << " collection not available" << std::endl;
      colMCPT = NULL;
    }
    
  //Define non-persistent variables

  if( colMCPT != NULL )
    {
      int mcptElements = colMCPT->getNumberOfElements();
      int jElements = colJet->getNumberOfElements();
     
      if(debug){

      	std::cout << "jElements = " << jElements << std::endl;
	std::cout << "mcptElements = " << mcptElements << std::endl;
	
      }
      
      jetPTn = {0};
      jetEn = {0};
      momSumMagSq = 0;
      momSumMagSqH = 0;
      momSumMagSqZ = 0;
      std::vector<float> jetPairMomentum;
      std::vector<float> jetPairEnergy;
      std::vector<float> jetPairEnergyHiggs;
      std::vector<float> jetPairEnergyZ;
      std::vector<float> bestGuessVecMom;
      //std::vector< std::vector<float> > momentumVectors;
      std::vector<float> momentumVectors;
      std::vector<float> ZMomentumVectors;
      std::vector<float> HiggsMomentumVectors;
      std::vector<float> angleVec;
      //START LOOP OVER PARTIClES
      for(int j = 0; j < mcptElements; j++)
	{
	 
	  BestGuessAnglemcpt = 100.0;
	  BestGuesspt = 10.0;
	  BestGuessEnergy = 0.0;

	  MCParticle* mcpt = dynamic_cast<MCParticle*>(colMCPT->getElementAt(j));
	  //if(abs(mcpt->getPDG())==23){_nZ++;}
	  //if(abs(mcpt->getPDG())==25){_nHiggs++;}
	  if ( (abs(mcpt->getPDG())<6 && mcpt->getParents().size() > 0) && mcpt->isCreatedInSimulation() == false && jElements > 3) // Was "== 4"
	    {
	      if (abs((mcpt->getParents())[0]->getPDG())==23 ||  abs((mcpt->getParents())[0]->getPDG())==25)
		{ 
		  //if(abs((mcpt->getParents())[0]->getPDG())==23){_nZQuarks++;}
		  //if(abs((mcpt->getParents())[0]->getPDG())==25){_nHiggsQuarks++;}
		  mcptPMag = sqrt(pow(mcpt->getMomentum()[0],2)+pow(mcpt->getMomentum()[1],2)+pow(mcpt->getMomentum()[2],2));
		  //std::cout<<"MC QUARK PARTICLE PDG = "<<mcpt->getPDG()<<std::endl;
		  //std::cout<<"Energy: "<<mcpt->getEnergy()<<"GeV"<<std::endl;
		  //std::cout<<"Z inv mass = "<<sqrt(pow(mcpt->getEnergy(),2)+pow(mcptPMag,2))<<"GeV"<<std::endl;

		  //START LOOP OVER JETS
		  for (int l = 0; l < jElements; l++)
		    {
		      ReconstructedParticle* thisjet = dynamic_cast<ReconstructedParticle*>(colJet->getElementAt(l));
		      DotProductmcpt = std::inner_product(mcpt->getMomentum(),mcpt->getMomentum()+3,thisjet->getMomentum(),0.0);
		      thisjetMag = sqrt(pow(thisjet->getMomentum()[0],2) + pow(thisjet->getMomentum()[1],2) + pow(thisjet->getMomentum()[2],2));
		      ThisAnglemcpt = acos(DotProductmcpt/(mcptPMag*thisjetMag));
		      if (ThisAnglemcpt< BestGuessAnglemcpt)// && fabs(mcptPMag-thisjetMag) <= BestGuesspt)
			{
			  BestGuessAnglemcpt=ThisAnglemcpt;
			  BestGuesspt =thisjetMag; // Best Guess jet absolute Momentum
			  BestGuessEnergy = thisjet->getEnergy(); // Best Guess jetEnergy
			  for(int t=0; t < 3 ; t++)
			    { 
			      bestGuessVecMom.push_back(thisjet->getMomentum()[t]);
			    }
			  //bestGuessVecMom.push_back(thisjet->getMomentum()); //Best Guess 3 vector momentum
			  //BestGuesspt = sqrt(pow(thisjet->getMomentum()[0],2)+pow(thisjet->getMomentum()[1],2)+pow(thisjet->getMomentum()[2],2)); //Transverse Momentum
			}
		       
		    }
		  _jetsbestGuesspt->Fill(BestGuessAnglemcpt);
		  if (abs((mcpt->getParents())[0]->getPDG())==25)
		    {
		      _jetsBestGuessAngleHiggs->Fill(BestGuessAnglemcpt);
		    }
		  if (abs((mcpt->getParents())[0]->getPDG())==23)
		    {
		      _jetsBestGuessAngleZ->Fill(BestGuessAnglemcpt);
		    }

		  /////////////////////////////////////////////// to work this uncomment 525-542  //start
		  //
		  //if (BestGuessAnglemcpt < 0.05 && abs((mcpt->getParents())[0]->getPDG())==23)//if best guess angle is less than arbitrary value (may be changed if necessary) get jet energy Z
		  //     {
		  //    //jetPairMomentum.push_back(BestGuesspt);
		  //    jetPairEnergyZ.push_back(BestGuessEnergy);
		  //    for(int y=bestGuessVecMom.size()-3;y<bestGuessVecMom.size();y++) { ZMomentumVectors.push_back(bestGuessVecMom[y]); }
		  //  }
		  //if (BestGuessAnglemcpt < 0.05 && abs((mcpt->getParents())[0]->getPDG())==25)//if best guess angle is less than arbitrary value (may be changed if necessary) get jet energy Higgs
		  //  {
		  //    jetPairEnergyHiggs.push_back(BestGuessEnergy);
		  //    for(int y=bestGuessVecMom.size()-3;y<bestGuessVecMom.size();y++) { HiggsMomentumVectors.push_back(bestGuessVecMom[y]); }
		  //  }
		  //angleVec.push_back(BestGuessAnglemcpt);  //stop

		  //-//-//-//-//  

		  if (abs((mcpt->getParents())[0]->getPDG())==23)//if best guess angle is less than arbitrary value (may be changed if necessary) get jet energy Z
		    {
		      //jetPairMomentum.push_back(BestGuesspt);
		      jetPairEnergyZ.push_back(BestGuessEnergy);
		      for(int y=bestGuessVecMom.size()-3 ; y<bestGuessVecMom.size();y++) { ZMomentumVectors.push_back(bestGuessVecMom[y]); } // WARNING: comparison between signed and unsigned int
		    }
		  if (abs((mcpt->getParents())[0]->getPDG())==25)//if best guess angle is less than arbitrary value (may be changed if necessary) get jet energy Higgs
		    {
		      jetPairEnergyHiggs.push_back(BestGuessEnergy);
		      for(int y=bestGuessVecMom.size()-3;y<bestGuessVecMom.size();y++) { HiggsMomentumVectors.push_back(bestGuessVecMom[y]); } // WARNING: comparison between signed and unsigned int
		    }
		      
		  angleVec.push_back(BestGuessAnglemcpt);
		
		  /////////////////////////////////////////////////

		}
	    }
	  
	}//PARTICLE LOOP CLOSE BRAKET 
     
      
      if (angleVec.size()>0)
      	{
	  _jetPairSep->Fill(*std::min_element(angleVec.begin(),angleVec.end())+*std::max_element(angleVec.begin(),angleVec.end()));
	  if(*std::min_element(angleVec.begin(),angleVec.end()) < 0.05 && *std::max_element(angleVec.begin(),angleVec.end()) < 0.1)
	    {
	      if (HiggsMomentumVectors.size() >5 )
		{
		  momSumMagSqH=pow(HiggsMomentumVectors[0]+HiggsMomentumVectors[3],2)+pow(HiggsMomentumVectors[1]+HiggsMomentumVectors[4],2)+pow(HiggsMomentumVectors[2]+HiggsMomentumVectors[5],2);
		  _jetOptH->Fill(sqrt(pow(jetPairEnergyHiggs[0]+jetPairEnergyHiggs[1],2)-momSumMagSqH));
		}
	      if (ZMomentumVectors.size() >5)
		{
		  momSumMagSqZ=pow(ZMomentumVectors[0]+ZMomentumVectors[3],2)+pow(ZMomentumVectors[1]+ZMomentumVectors[4],2)+pow(ZMomentumVectors[2]+ZMomentumVectors[5],2);
		  _jetOptZ->Fill(sqrt(pow(jetPairEnergyZ[0]+jetPairEnergyZ[1],2)-momSumMagSqZ));
		}
	    } 
	}

      
      // this is all old code we kept around just in case; ignore it

      //if (HiggsMomentumVectors.size() >5 ) //to wrok this uncomment 465-476 //start
      //  {
      //  std::cout<<"HIGGS"<<std::endl;
      //  momSumMagSqH=pow(HiggsMomentumVectors[0]+HiggsMomentumVectors[3],2)+pow(HiggsMomentumVectors[1]+HiggsMomentumVectors[4],2)+pow(HiggsMomentumVectors[2]+HiggsMomentumVectors[5],2);
      //  std::cout<<"Higgs energy size"<< jetPairEnergyHiggs.size() <<std::endl;
      //  _jetOptH->Fill(sqrt(pow(jetPairEnergyHiggs[0]+jetPairEnergyHiggs[1],2)-momSumMagSqH));
      //
      //  //std::cout<<"Vector Size = "<<momentumVectors.size()<<std::endl;
      //  // momSumMagSq=pow(momentumVectors[0]+momentumVectors[3],2)+pow(momentumVectors[1]+momentumVectors[4],2)+pow(momentumVectors[2]+momentumVectors[5],2);
      //  //_jetMinP->Fill(sqrt(pow(jetPairEnergy[0]+jetPairEnergy[1],2)-momSumMagSq));
      //}
      //if (ZMomentumVectors.size() >5)
      //{
      //  std::cout<<"Z"<<std::endl;
      //  momSumMagSqZ=pow(ZMomentumVectors[0]+ZMomentumVectors[3],2)+pow(ZMomentumVectors[1]+ZMomentumVectors[4],2)+pow(ZMomentumVectors[2]+ZMomentumVectors[5],2);
      //  std::cout<<"Z energy size"<< jetPairEnergyZ.size() <<std::endl;
      //  _jetOptZ->Fill(sqrt(pow(jetPairEnergyZ[0]+jetPairEnergyZ[1],2)-momSumMagSqZ));
      //}  //stop



      //if (jetPairMomentum.size() > 0)
      //{
      //std::cout<<"Best momentum Match stores "<<jetPairMomentum.size()<<std::endl;
      //_jetMinP->Fill(sqrt(pow(jetPairEnergy[0]+jetPairEnergy[1],2)-pow(jetPairMomentum[0]+jetPairMomentum[1],2)));      //POSSIBLY OUTDATED (CHECK WHOLE CODE TO BE SURE)//

      //	}
      



    }// LOOP CLOSE BRAKET //

  //for (int h=0; h<jElements ; h++)
  //	    {
  //	      ReconstructedParticle* thisjet = dynamic_cast<ReconstructedParticle*>(colJet->getElementAt(h));
  //	      jetPTn[h] = sqrt(pow(thisjet->getMomentum()[0],2) + pow(thisjet->getMomentum()[1],2));// TRANSVERSE MOMENTUM
  //	    }
  //	  for(int p=0; p< jElements ; p++)
  //	    {
  //	      // Opening jet objects
  //	      // ReconstructedParticle* thisjet = dynamic_cast<ReconstructedParticle*>(colJet->getElementAt(p));

  //jetPT[p] = sqrt(pow(thisjet->getMomentum()[0],2) + pow(thisjet->getMomentum()[1],2) + pow(thisjet->getMomentum()[2],2)); // MOMENTUM MAGNITUDE
  //jetPTn[p] = sqrt(pow(thisjet->getMomentum()[0],2) + pow(thisjet->getMomentum()[1],2) );// TRANSVERSE MOMENTUM 
  //	      for(int q = 0; q < jElements; q++)
  //		{
  //		  if (jElements == 4)
  //		    {
  //		      if (q != p)
  //			{
  //			  if(p < q)
  //			    { 
  //			      //invmass = sqrt(pow(jetE[p]+jetE[q],2)-pow(jetPT[p]+jetPT[q],2));
  //			      _jetMinP->Fill(jetPTn[p]+jetPTn[q]);
  //			    }
  //			}
  //		    }
  //		}
  //	    }	














  // //MCTRUTHLINK
  //  LCCollection* colMCTL = NULL;
  //  int pIDSize;
  //  // HiggsDaughterList = 0;
  //  
  //  try
  //    {
  //      colMCTL = evt->getCollection( _colMCTL );
  //    }
  //  catch( lcio::DataNotAvailableException e )
  //    {
  //      streamlog_out(WARNING) << _colMCTL << " collection not available" << std::endl;
  //      colMCTL = NULL;
  //   }
  //    
  //  //Define non-persistent variables
  //  
  //  if( colMCTL != NULL )
  //    {
  //      int nElements = colMCTL->getNumberOfElements();
  //      int nElements_MCP = colMCP->getNumberOfElements();
  //
  //      
  //      if(debug){
  //	std::cout << "nElements = " << nElements << std::endl;
  //     }
  //      
  //      
  //      //START LOOP OVER PARTICLE
  //     for(int i=0; i< nElements ; i++)
  //	{
  //	  //link MCParticle with RecoParticle
  //	  LCRelation* rel = dynamic_cast<LCRelation*>( colMCTL->getElementAt( i ) ) ;
  //	  MCParticle* mcptl = dynamic_cast<MCParticle*>( rel->getTo() ) ;
  //	  ReconstructedParticle* rptl = dynamic_cast<ReconstructedParticle*>( rel->getFrom() );
  //
  //	  // _RPlikelyhood->Fill(rptl->getGoodnessOfPID());
  //	  //_RPlikelyhood->Fill(rptl->getParticleIDs()[0]->getLikelihood());
  //
  //	  //std::cout <<"Momentum dot product: "<< DotProduct  <<std::endl;
  //	  //std::cout <<"Momentum magnitude of rptl: "<< ptRPtl <<std::endl;
  //	  //std::cout <<"Momentum magnitude of mcptl: "<< ptMCPtl <<std::endl;
  //	  
  //	  //std::cout <<"Angle: "<< acos(DotProduct/(ptMCPtl*ptRPtl)) <<std::endl;
  //	  
  //
  //	  BestGuessAngle = 100.0;
  //	  BestGuessMomentum = 0.0;
  //	  BestGuessPDG = 0;
  //	  BestGuessCharge = 0;
  //	  xDotProduct=0;
  //	  yDotProduct=0;
  //	  zDotProduct=0;
  //	  //BestGuessmcp= NULL;
  //	  
  //	  for(int j=0; j<nElements_MCP; j++)
  //	    {
  //	      MCParticle* mcp = dynamic_cast<MCParticle*>(colMCP->getElementAt(j));
  //	      
  //	      //ThisMomentum = mcp->getMomentum();
  //
  //	      rptlPMag = sqrt(pow(rptl->getMomentum()[0],2)+pow(rptl->getMomentum()[1],2)+pow(rptl->getMomentum()[2],2));
  //	      mcpPMag = sqrt(pow(mcp->getMomentum()[0],2)+pow(mcp->getMomentum()[1],2)+pow(mcp->getMomentum()[2],2));
  //	      
  //	      
  //	      xDotProduct =  std::inner_product(mcp->getMomentum(),mcp->getMomentum()+3,xAxis,0.0);
  //	      yDotProduct =  std::inner_product(mcp->getMomentum(),mcp->getMomentum()+3,yAxis,0.0);
  //	      zDotProduct =  std::inner_product(mcp->getMomentum(),mcp->getMomentum()+3,zAxis,0.0);
  //
  //	      DotProduct = std::inner_product(mcp->getMomentum(),mcp->getMomentum()+3,rptl->getMomentum(),0.0);
  //
  //	      ThisAngle = acos(DotProduct/(mcpPMag*rptlPMag));
  //
  //	       _angle->Fill(acos(DotProduct/(rptlPMag*mcpPMag)));
  //
  //	     
  //	       if( ThisAngle < BestGuessAngle && rptl->getCharge() == mcp->getCharge()  && mcp->isCreatedInSimulation () == false)// && ((abs(mcp->getPDG())>10 && abs(mcp->getPDG())<17 && (mcp->getPDG())%2==1) || abs(mcp->getPDG())==22 || abs(mcp->getPDG())>100))
  //		 {
  //		   if( (abs(mcp->getPDG())>10 && abs(mcp->getPDG())<17 && (mcp->getCharge()!= 0 ) || abs(mcp->getPDG())==22 || abs(mcp->getPDG())>100  ) )
  //		     {
  //		       BestGuessAngle = ThisAngle;
  //		       BestGuessMomentum = mcpPMag;
  //		       BestGuessPDG = mcp->getPDG();
  //		       BestGuessCharge = mcp->getCharge();
  //		       //BestGuessmcp = mcp;
  //		     }
  //		 }
  //	       // std::cout<<"start if"<<std::endl;
  //	       if (mcp->getDaughters().size()!=0)
  //		 {
  //		   if(abs(mcp->getPDG())==11 && abs((mcp->getDaughters())[0]->getPDG())==6)
  //		     { 
  //		       //std::cout<<"in if"<<std::endl;
  //		       _xangle->Fill(acos(xDotProduct/mcpPMag));
  //		       _yangle->Fill(acos(yDotProduct/mcpPMag));
  //		       _zangle->Fill(acos(zDotProduct/mcpPMag));
  //	    
  //		     }
  //		   //std::cout<<"end if"<<std::endl;
  //		 }
  //	    }
  //
  //	  //if(abs(mcp->getPDG())==11 && abs(mcp->getDaughters())==6)
  //	  //{
  //	  //  _xangle->Fill(acos(xDotProduct/mcpPMag));
  //	  //  _yangle->Fill(acos(yDotProduct/mcpPMag));
  //	  //  _zangle->Fill(acos(zDotProduct/mcpPMag));
  //	  // }
  //
  //
  //
  //	  //_xangle->Fill(acos(xDotProduct/BestGuessMomentum));
  //	  //_yangle->Fill(acos(yDotProduct/BestGuessMomentum));
  //	  //_zangle->Fill(acos(zDotProduct/BestGuessMomentum));
  //
  //
  //	 
  //	  if(BestGuessAngle < 0.0001 && ((abs(BestGuessPDG) > 10 && abs(BestGuessPDG) < 17 && BestGuessCharge != 0) || BestGuessPDG==22  || abs(BestGuessPDG)>100) )// && sqrt(pow(rptl->getMomentum()[0],2)+pow(rptl->getMomentum()[1],2)+pow(rptl->getMomentum()[2],2)) > 0.01*BestGuessMomentum)
  //	    {
  //	      // std::cout << "== ORIGINAL PARTICLE ==" << std::endl;
  //	      // std::cout << "Angle: " << acos(std::inner_product(mcptl->getMomentum(),mcptl->getMomentum()+3,rptl->getMomentum(),0.0)/( sqrt(pow(rptl->getMomentum()[0],2)+pow(rptl->getMomentum()[1],2)+pow(rptl->getMomentum()[2],2))*sqrt(pow(mcptl->getMomentum()[0],2)+pow(mcptl->getMomentum()[1],2)+pow(mcptl->getMomentum()[2],2)))) << std::endl;
  //	      // std::cout << "Momentum: " << sqrt(pow(rptl->getMomentum()[0],2)+pow(rptl->getMomentum()[1],2)+pow(rptl->getMomentum()[2],2)) << std::endl;
  //	      // std::cout << "Charge: " << rptl->getCharge() << std::endl;
  //	      //std::cout << "PDG: " << rptl->getParticleIDs()[0]->getPDG()  << std::endl;
  //	  
  //	      // std::cout << "== BEST GUESS ==" << std::endl;
  //	      //std::cout << "Angle: " << BestGuessAngle << std::endl;
  //	      //std::cout << "Momentum: " << BestGuessMomentum << std::endl;
  //	      //std::cout << "Charge: " << BestGuessCharge << std::endl;
  //	      // std::cout << "PDG: " << BestGuessPDG << std::endl;
  //	  
  //
  //	      
  //	      //////////HISTOGRAMS//////////
  //	      _bestGuess->Fill(BestGuessPDG);//PDG of generated particles (MC)
  //	      _angleBestGuess->Fill(BestGuessAngle);//angles between generated (MC) particle and source (rptl) particle for every particle
  //	      _ptRPtl->Fill(sqrt(pow(rptl->getMomentum()[0],2)+pow(rptl->getMomentum()[1],2)+pow(rptl->getMomentum()[2],2)));//Momentum of source (rptl) particle
  //	      _ptMCPtl->Fill(sqrt(pow(mcptl->getMomentum()[0],2)+pow(mcptl->getMomentum()[1],2)+pow(mcptl->getMomentum()[2],2)));//Momentum of generated (MC) particle
  //	      //////////////////////////////
  //	  
  //	      //if (abs(BestGuessPDG)>10 && abs(BestGuessPDG<15))
  //	      // {
  //	      //   _ptBestGuess->Fill(BestGuessMomentum); //Best guess lepton momentum (MC)
  //	      // }
  //	  
  //	      
  //	  
  //	      if ((abs(BestGuessPDG) > 10 && abs(BestGuessPDG) < 17 && BestGuessCharge != 0)  )
  //		{
  //		  _generated_MCPtl_pt->Fill(BestGuessMomentum); //Besst Guess lepton momentum excluding neutrinos (MC)
  //		  _generated_RPtl_pt->Fill(sqrt(pow(rptl->getMomentum()[0],2)+pow(rptl->getMomentum()[1],2)+pow(rptl->getMomentum()[2],2)));//source leptons excluing neutrinos momentum (rptl)
  //		  _ptDiffBestGuess->Fill((sqrt(pow(rptl->getMomentum()[0],2)+pow(rptl->getMomentum()[1],2)+pow(rptl->getMomentum()[2],2))-BestGuessMomentum)/BestGuessMomentum);//
  //		  
  //		  
  //		  if  (abs(BestGuessPDG)==15) {std::cout << "Tau in the matchloop" << std::endl;}
  //
  //		   
  //		  if (abs(mcptl->getPDG()) == 15 && abs(mcptl->getParents()[0]->getPDG())==25)
  //		    {
  //		      std::cout<<"MAIN LOOP TRIGERED"<<std::endl;
  //		      //HiggsDaughterList = mcptl->getParents()[0]->getDaughters();
  //		      for (int k=0; k <  (mcptl->getParents()[0]->getDaughters()).size() ;k++ )
  //		  	{
  //			  std::cout<<"SECONDARY LOOP TRIGGERED"<<std::endl;
  //			  if((mcptl->getParents()[0]->getDaughters())[k]->getPDG()==15) 
  //			    {
  //			      std::cout<<"==TAU=="<<std::endl;
  //			    }
  //
  //			  
  //		  	}
  //		    }
  //		}
  //	    
  //	      else if (BestGuessAngle < 99.0)// || sqrt(pow(rptl->getMomentum()[0],2)+pow(rptl->getMomentum()[1],2)+pow(rptl->getMomentum()[2],2)) < 0.01*BestGuessMomentum ) 
  //		{
  //		  _ptDiffBadGuess->Fill((sqrt(pow(rptl->getMomentum()[0],2)+pow(rptl->getMomentum()[1],2)+pow(rptl->getMomentum()[2],2))-BestGuessMomentum)/BestGuessMomentum);// 
  //
  //		  
  //		}
  //	      else
  //		{
  //		  std::cout << "No match found for particle!" << std::endl;
  //		  ++NoMatchParticles;
  //		}
  //	    }
  //	  // if (mcptl->isCreatedInSimulation () == false )
  //	  // {
  //	  //  if (BestGuessPDG>10 && BestGuessPDG<15)
  //	  //	{
  //	  //	  _generated_MCPtl_pt->Fill(BestGuessMomentum);
  //	  //	  _generated_RPtl_pt->Fill(sqrt(pow(rptl->getMomentum()[0],2)+pow(rptl->getMomentum()[1],2)+pow(rptl->getMomentum()[2],2)));
  //	  //	}
  //	  //}
  //
  //	  if( rptl->getParticleIDs().size()>0)
  //	    {
  //	      //pIDSize = rptl->getParticleIDs().size();
  //	      //std::cout << "ReconstructedParticle PDG(): " << rptl->getParticleIDs()[pIDSize-1]->getPDG() << std::endl; //Least likely PDG
  //	      //std::cout << "all MC part. PDG() : " << mcptl->getPDG() << std::endl; //MC particle PDG
  //	      //std::cout << "all ReconstructedParticle PDG(): " << rptl->getParticleIDs()[0]->getPDG() << std::endl; //most likely particle PDG
  //	      //std::cout << "all ReconstructedParticle charge: " << rptl->getCharge() << std::endl; //most likely particle charge
  //	      // std::cout << "ReconstructedParticle Type: " << rptl->getType() << std::endl; //RP type
  //	      //for(int o=0; o< rptl->getParticleIDs().size() ; o++)
  //	      //{
  //	      //  if (rptl->getParticleIDs()[o]->getPDG() == mcptl->getPDG())
  //	      //    {
  //	      //      std::cout<< "GetParticleIDs constains MC particle" << std::endl;
  //	      //    }
  //	      //}
  //	  
  //	      if(abs(mcptl->getPDG())<2300)
  //		{
  //		  //std::cout << "MC part. PDG() < 2300 : " <<  mcptl->getPDG() << std::endl; //MC particle PDG
  //		  _ptMCPDG->Fill((mcptl->getPDG()));//fill histogram of MC particles PDGs for leptons and bosons
  //		  //std::cout << "1ReconstructedParticle PDG(): " << rptl->getParticleIDs()[0]->getPDG() << std::endl; //most likely particle PDG
  //		}
  //	      else
  //		{
  //		  _ptMCPDG->Fill(0);
  //		}
  //
  //	      if(abs(rptl->getParticleIDs()[0]->getPDG())<2300)
  //		{
  //		  if(rptl->getParticleIDs()[0]->getPDG() < 14)
  //		    {
  //		      _ptRPLeptons->Fill(abs(*(rptl->getMomentum())));//fill RP histogram
  //		      if(rptl->getCharge() == -1)
  //			{
  //			  _ptRPPDG->Fill(rptl->getParticleIDs()[0]->getPDG());
  //			}  
  //		      else
  //			{
  //			  _ptRPPDG->Fill(-(rptl->getParticleIDs()[0]->getPDG())) ;
  //			}
  //		    }
  //		
  //		  else
  //		    {
  //		      //std::cout << "2ReconstructedParticle PDG(): " << rptl->getParticleIDs()[0]->getPDG() << std::endl; //most likely particle PDG
  //		      _ptRPPDG->Fill((rptl->getParticleIDs()[0]->getPDG()));//fill RP histogram of PDGs
  //		    }
  //		}	
  //	      else
  //		{
  //		  _ptRPPDG->Fill(0);
  //		}
  //	      
  //	      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //	      // if(abs(rptl->getType())<2300)
  //	      //{
  //	      //  //std::cout << "2ReconstructedParticle PDG(): " << rptl->getParticleIDs()[0]->getPDG() << std::endl; //most likely particle PDG
  //	      //  _ptRPType->Fill(rptl->getType());//fill RP histogram of PDGs
  //	      //}
  //	      //     else
  //	      //{
  //	      //  _ptRPType->Fill(0);
  //	      //}
  //	      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //	      if( abs((rptl->getParticleIDs()[0]->getPDG())==25) || abs((rptl->getParticleIDs()[0]->getPDG())==23))
  //		{
  //		  std::cout<<"Higgs or Z"<<std::endl;
  //		  _massRPV->Fill((rptl->getMass())); // fill MPC V particles mass (higgs and Z) histogram
  //		  //std::cout << "Parent PDG: " << mcp->getParents()[0] << std::endl;
  //		}
  //	    }
  //	  if( mcptl->getParents().size()>0 ) //if MCP has a parent
  //	    {
  //	      //std::cout << "PDG: " << abs(mcptl->getPDG()) << "\t\tParent PDG: " << abs(mcptl->getParents()[0]->getPDG()) << "\t\tGrandparent PDG: " << abs(mcptl->getParents()[0]->getParents()[0]->getPDG()) << "\t\tGGparent PDG: " << abs(mcptl->getParents()[0]->getParents()[0]->getParents()[0]->getPDG()) << std::endl;
  //	      if( rptl->getParticleIDs().size()>0)
  //		{
  //		  //std::cout << "ReconstructedPartile getType() returns: " << abs(rptl->getType()) << std::endl;
  //		  //std::cout << "ReconstructedPartile getParticleIDs()[0]->getPDG() returns: " << rptl->getParticleIDs()[0]->getPDG() << std::endl;
  //		  //std::cout << "ReconstructedPartile getParticleIDs()[0]->getPDG() returns: " << (rptl->getParticleIDs()[0]->getPDG()) << std::endl;
  //		  // std::cout << "PDG MCPTL" << mcptl->getParents()[0]->getPDG() << std::endl;
  //		  if(abs(mcptl->getParents()[0]->getPDG())==24) //if MCP parent is a W
  //		    {
  //		      if(mcptl->getParents()[0]->getParents().size()>0) //if MCP has a grandparent
  //			{
  //		  
  //			  if(abs(mcptl->getParents()[0]->getParents()[0]->getPDG())==6) //if MCP grandparent is a top
  //			    {
  //			      if((abs(mcptl->getPDG())>10) && (abs(mcptl->getPDG())<17))//If MCP is a lepton
  //				{
  //				  // if( abs(rptl->getType())==11 || abs(rptl->getType())==12 || abs(rptl->getType())==13 || abs(rptl->getType())==14 || abs(rptl->getType())==15 || abs(rptl->getType())==16)//If RP is a lepton
  //				  if(abs(rptl->getParticleIDs()[0]->getPDG())>10 && abs(rptl->getParticleIDs()[0]->getPDG())<17 )//If RP is a lepton
  //				    {
  //				      _ptRPLeptons->Fill(sqrt(pow(rptl->getMomentum()[0],2)+pow(rptl->getMomentum()[1],2)+pow(rptl->getMomentum()[2],2)));//fill RP histogram
  //				      //std::cout<<"Momentum of Lepton :"<< rptl->getMomentum()  <<std::endl;
  //				    }
  //				}
  //			    }
  //			}
  //		    }
  //		}
  //	    }
  //	}
  //    //END LOOP OVER PARTICLE
  //
  //    }
  //
  //  //  if(debug)
  //  //    {
  //  //std::cout<<"Number of Higgs "<<NumberHiggs<<std::endl;
  //  //std::cout<<"Number of leptons"<<NumLeptons<<std::endl;
  //      //std::cout<<"Number of semileptonic decays "<<NumSemileptonic<<std::endl;
  //      //      std::cout<<"Number of hadronic decays "<<NumHadronic<<std::endl;
  //      //   }
  //
  //
  // //fill histograms 
  //  // _ndecaytype->Fill(decaytype);
  





  /////////////////////////////////////////////////////TESTING AREA FOR RECONSTRUCTION OF LEPTONS' INVARAINT MASS FOR Z AND HIGGS///////////////////////////////////////////////////////////////////








  //MCTRUTHLINK
  LCCollection* colMCTL = NULL;
  int pIDSize;
  // HiggsDaughterList = 0;
  std::vector<float> lepPx;
  std::vector<float> lepPy;
  std::vector<float> lepPz;
  std::vector<float> lepE;
  try
    {
      colMCTL = evt->getCollection( _colMCTL );
    }
  catch( lcio::DataNotAvailableException e )
    {
      streamlog_out(WARNING) << _colMCTL << " collection not available" << std::endl;
      colMCTL = NULL;
    }
    
  //Define non-persistent variables

  if( colMCTL != NULL )
    {
      int nElements = colMCTL->getNumberOfElements();
      int nElements_MCP = colMCP->getNumberOfElements();

      
      if(debug){
	std::cout << "nElements = " << nElements << std::endl;
      }
      

      //START LOOP OVER PARTICLE
      for(int i=0; i< nElements ; i++)
	{
	  //link MCParticle with RecoParticle
	  LCRelation* rel = dynamic_cast<LCRelation*>( colMCTL->getElementAt( i ) ) ;
	  MCParticle* mcptl = dynamic_cast<MCParticle*>( rel->getTo() ) ;
	  ReconstructedParticle* rptl = dynamic_cast<ReconstructedParticle*>( rel->getFrom() );

	  BestGuessAngle = 100.0;
	  BestGuessMomentum = 0.0;
	  BestGuessPDG = 0;
	  BestGuessCharge = 0;
	  xDotProduct=0;
	  yDotProduct=0;
	  zDotProduct=0;
	  //BestGuessmcp= NULL;
	  BestGuessParent = 0;

	  for(int j=0; j<nElements_MCP; j++)
	    {
	      MCParticle* mcp = dynamic_cast<MCParticle*>(colMCP->getElementAt(j));
	      
	      
	      rptlPMag = sqrt(pow(rptl->getMomentum()[0],2)+pow(rptl->getMomentum()[1],2)+pow(rptl->getMomentum()[2],2));
	      mcpPMag = sqrt(pow(mcp->getMomentum()[0],2)+pow(mcp->getMomentum()[1],2)+pow(mcp->getMomentum()[2],2));
	      
	      
	      xDotProduct =  std::inner_product(mcp->getMomentum(),mcp->getMomentum()+3,xAxis,0.0);
	      yDotProduct =  std::inner_product(mcp->getMomentum(),mcp->getMomentum()+3,yAxis,0.0);
	      zDotProduct =  std::inner_product(mcp->getMomentum(),mcp->getMomentum()+3,zAxis,0.0);

	      DotProduct = std::inner_product(mcp->getMomentum(),mcp->getMomentum()+3,rptl->getMomentum(),0.0);

	      ThisAngle = acos(DotProduct/(mcpPMag*rptlPMag));

	      _angle->Fill(acos(DotProduct/(rptlPMag*mcpPMag)));

	     
	      if( ThisAngle < BestGuessAngle && rptl->getCharge() == mcp->getCharge()  && mcp->isCreatedInSimulation () == false)
		{
		  if( (abs(mcp->getPDG())>10 && abs(mcp->getPDG())<17 && (mcp->getCharge()!= 0 ) || abs(mcp->getPDG())==22 || abs(mcp->getPDG())>100  ))
		    {
		      BestGuessAngle = ThisAngle;
		      BestGuessMomentum = mcpPMag;
		      BestGuessPDG = mcp->getPDG();
		      BestGuessCharge = mcp->getCharge();
		      if(mcp->getParents().size()!=0)
			{
			  BestGuessParent = mcp->getParents()[0]->getPDG();
			}
		    }
		}
	      
	      if (mcp->getDaughters().size()!=0)
		{
		  if(abs(mcp->getPDG())==11 && abs((mcp->getDaughters())[0]->getPDG())==6)
		    { 
		      _xangle->Fill(acos(xDotProduct/mcpPMag));
		      _yangle->Fill(acos(yDotProduct/mcpPMag));
		      _zangle->Fill(acos(zDotProduct/mcpPMag));
		    }
		   
		}
	    }
	 
	  // std::cout<<"parents PDG: "<<BestGuessParent<<std::endl;
	  
	  if((abs(BestGuessPDG) > 10 && abs(BestGuessPDG) < 17 && BestGuessCharge != 0)  && (abs(BestGuessParent)==23 || abs(BestGuessParent)==25)) 
	    {
	      //std::cout<<"lepton "<<BestGuessPDG<<" with boson parents"<<std::endl;
	      //std::cout<<"x mom= "<<rptl->getMomentum()[0]<<std::endl;
	      //std::cout<<"y mom= "<<rptl->getMomentum()[1]<<std::endl;
	      //std::cout<<"z mom= "<<rptl->getMomentum()[2]<<std::endl;
	      //std::cout<<"energy= "<<rptl->getEnergy()<<std::endl;
	     
	      
	      lepPx.push_back(rptl->getMomentum()[0]);
	      lepPy.push_back(rptl->getMomentum()[1]);
	      lepPz.push_back(rptl->getMomentum()[2]);
	      lepE.push_back(rptl->getEnergy());

	      //_leptonInvmass->Fill(sqrt(pow(rptl->getEnergy(),2)-(pow(rptl->getMomentum()[0],2)+pow(rptl->getMomentum()[1],2)+pow(rptl->getMomentum()[2],2))));	   
	    }

	  if(BestGuessAngle < 0.0001 && ((abs(BestGuessPDG) > 10 && abs(BestGuessPDG) < 17 && BestGuessCharge != 0) || abs(BestGuessPDG)==22  || abs(BestGuessPDG)>100)) 
	    {
	      //////////HISTOGRAMS//////////
	      _bestGuess->Fill(BestGuessPDG);//PDG of generated particles (MC)
	      _angleBestGuess->Fill(BestGuessAngle);//angles between generated (MC) particle and source (rptl) particle for every particle
	      _ptRPtl->Fill(sqrt(pow(rptl->getMomentum()[0],2)+pow(rptl->getMomentum()[1],2)+pow(rptl->getMomentum()[2],2)));//Momentum of source (rptl) particle
	      _ptMCPtl->Fill(sqrt(pow(mcptl->getMomentum()[0],2)+pow(mcptl->getMomentum()[1],2)+pow(mcptl->getMomentum()[2],2)));//Momentum of generated (MC) particle
	      //////////////////////////////
	  
	      if ((abs(BestGuessPDG) > 10 && abs(BestGuessPDG) < 17 && BestGuessCharge != 0)  )
		{
		  _generated_MCPtl_pt->Fill(BestGuessMomentum); //Besst Guess lepton momentum excluding neutrinos (MC)
		  _generated_RPtl_pt->Fill(sqrt(pow(rptl->getMomentum()[0],2)+pow(rptl->getMomentum()[1],2)+pow(rptl->getMomentum()[2],2)));//source leptons excluing neutrinos momentum (rptl)
		  _ptDiffBestGuess->Fill((sqrt(pow(rptl->getMomentum()[0],2)+pow(rptl->getMomentum()[1],2)+pow(rptl->getMomentum()[2],2))-BestGuessMomentum)/BestGuessMomentum);//
		  
		}
	    
	      else if (BestGuessAngle < 99.0)// || sqrt(pow(rptl->getMomentum()[0],2)+pow(rptl->getMomentum()[1],2)+pow(rptl->getMomentum()[2],2)) < 0.01*BestGuessMomentum ) 
		{
		  _ptDiffBadGuess->Fill((sqrt(pow(rptl->getMomentum()[0],2)+pow(rptl->getMomentum()[1],2)+pow(rptl->getMomentum()[2],2))-BestGuessMomentum)/BestGuessMomentum);// 
		}
	      else
		{
		  std::cout << "No match found for particle!" << std::endl;
		  ++NoMatchParticles;
		}
	    }
	  //if( abs((rptl->getParticleIDs()[0]->getPDG())==25) || abs((rptl->getParticleIDs()[0]->getPDG())==23))
	  //	{
	  //	  std::cout<<"Higgs or Z"<<std::endl;
	  //	  _massRPV->Fill((rptl->getMass())); // fill MPC V particles mass (higgs and Z) histogram
	  //	  
	  //	}
	
	} //END LOOP OVER PARTICLE
    }
  lepInvMass = 0.0;
  xs = 0.0;
  ys = 0.0;
  zs = 0.0;
  Etot = 0.0;
  bestGuessLepInvMass = 10000.0;
  singLepInvMass = 10001.0;
   
  if(lepPx.size()!=0)
    {
      for(int a = 0; a<lepPx.size();a++) // WARNING: comparison between signed and unsigned int
	{
	  singLepInvMass = sqrt((pow(lepE[a],2)-pow(fabs(lepPx[a]),2)+pow(fabs(lepPy[a]),2)+pow(fabs(lepPz[a]),2))); 
	  
	  for (int b=0;b<lepPx.size();b++) // WARNING: comparison between signed and unsigned int
	    {
	      if (b>a)
		{
		  xs = pow(lepPx[b] + lepPx[a],2);
		  ys = pow(lepPy[b] + lepPy[a],2);
		  zs = pow(lepPz[b] + lepPz[a],2);
		  Etot = lepE[b] + lepE[a];
		  lepInvMass = sqrt(pow(Etot,2)-(xs+ys+zs));
		  if (fabs(lepInvMass-91.1)<fabs(bestGuessLepInvMass-91.1) || fabs(lepInvMass-125.1)<fabs(bestGuessLepInvMass-125.1) )
		    {
		      bestGuessLepInvMass = lepInvMass;
		    }
		}
	    }
	  if (fabs(singLepInvMass-91.1)<fabs(bestGuessLepInvMass-91.1) || fabs(singLepInvMass-125.1)<fabs(bestGuessLepInvMass-125.1))
	    {
	      bestGuessLepInvMass = singLepInvMass;
	    }
	}
      if(bestGuessLepInvMass < 10000.0)
	{
	  std::cout<<"Leptons invariant mass = "<<bestGuessLepInvMass<<std::endl;
	  _leptonInvmass->Fill(bestGuessLepInvMass);
	}
    }















  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  

 
 
}



void SignalSeparator::check( LCEvent * evt ) 
{ 
  // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void SignalSeparator::end()
{ 

  //std::cout << "Number of Higgs bosons: " << NumberHiggs << std::endl;

  std::cout << "Total particles with no match: " << NoMatchParticles << std::endl;

  std::cout << "SignalSeparator::end()  " << name() 
	    << " processed " << _nEvt << " events in " << _nRun << " runs "
	    << std::endl ;

  std::cout <<"Number of Higgs bosons = " <<_nHiggs<<std::endl;
  std::cout <<"Number of Quarks decayed from Higgs = "<<_nHiggsQuarks<<std::endl;
  std::cout <<"Number of Z bosons = " <<_nZ<<std::endl;
  std::cout <<"Number of Quarks decayed from Z = "<<_nZQuarks<<std::endl;
	   
  //std::cout<<"Number of Higgs bosons: "<< NumberHiggs <<std::endl;
  //std::cout<<"Number of Z bosons: "<< NumberHiggs <<std::endl;

}






