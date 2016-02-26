#include "SignalSeparator.h"
#include <iostream>
#include <math.h>
#include <vector>

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

bool mydebug = false;
bool debug = false;




SignalSeparator aSignalSeparator ;


SignalSeparator::SignalSeparator() : Processor("SignalSeparator") 
{

  // modify processor description
  _description = "SignalSeparator looks at a collection of input files and seperates them into different types of decay such as WW->leptonic and WW->hadronic. By varying \"choice\" you can choose which events to let pass- 1=semileptonic 2=leptonic 3=hadronic 4= other";
  _description = "New version of SignalSeparator that looks at a collection of input files and seperates them into differnt types of decay such as WW->leptonic and ww->hadronic, but customised specifically for the ttV process. By varying \"choice\" you can chose which events to let pass= 1=?? 2=?? 3=?? 4=?? 5=??"


  // register steering parameters: name, description, class-variable, default value
  registerInputCollection( LCIO::MCPARTICLE,
			   "CollectionName" , 
			   "Name of the MCParticle collection"  ,
			   _colName ,
			   //			   std::string("MCParticlesSkimmed")
			   std::string("MCParticle")
			   );

  registerProcessorParameter( "Choice",
			      "Choice of process to allow through",
			      _choice,
			      int(1));

}

void SignalSeparator::init()
{ 

  streamlog_out(DEBUG) << "   init called  " << std::endl ;

  // usually a good idea to
  printParameters() ;

  _nRun = 0 ;
  _nEvt = 0 ;
  nskipped=0;
  npassed=0;

  AIDA::IHistogramFactory* pHistogramFactory=marlin::AIDAProcessor::histogramFactory( this );
  AIDA::ITree* pTree=marlin::AIDAProcessor::tree( this );
  
  if(  pHistogramFactory!=0 )
    {
      if (!(pTree->cd( "/" + name() + "/"))) 
	{
	  pTree->mkdir( "/" + name() + "/" );
	  pTree->cd( "/" + name() + "/");
	}
			
        _nleptons = new TH1F("nleptons", "Number of leptons coming from  W decay", 4, -0.5, 3.5);
      // _nWDecays = new TH1F("nWDecays", "Number of W particles from the H-decay", 10, -0.5, 9.5);
      // _ndecayproducts = new TH1F("ndecayproducts", "Number of decay products from each W", 50, -0.5, 49.5);
      //_ndecayPDG = new TH1F("decayPDG", "PDG code for products of W decay", 150,-0.5, 299.5); 
      //_nleptonPDG = new TH1F("leptonPDG", "PDG of leptons", 20, -0.5, 19.5);
	_ndecaytype = new TH1F("decaytype", "Decay type label", 5, -0.5, 4.5);
	_nParticleLeptonic = new TH1F("nParticleLeptonic", "Number of MCP Particles in event", 500, 0.0, 500);
	_nParticleSemiLeptonic = new TH1F("nParticleSemiLeptonic", "Number of MCP Particles in event", 500, 0.0, 500);
	_nParticleHadronic  = new TH1F("nParticleHadronic", "Number of MCP Particles in event", 500, 0.0, 500);
	_nParticleUnclassified  = new TH1F("nParticleUnclassified", "Number of MCP Particles in event", 500, 0.0, 500);
      	

    }
}

void SignalSeparator::processRunHeader( LCRunHeader* run) { 

  _nRun++ ;
} 



void SignalSeparator::processEvent( LCEvent * evt ) 
{ 


  // LCTOOLS::dumpEvent( evt )                  // not sure entirely what this does, but it seems to make a lot more print out in terminal

  LCCollection* col = NULL;
  
  try
    {
      col = evt->getCollection( _colName );
    }
  catch( lcio::DataNotAvailableException e )
    {
      streamlog_out(WARNING) << _colName << " collection not available" << std::endl;
      col = NULL;
    }
    
  // make sure all variables start as zero
  // Added integer for number of Z bosons.
  int WLeptonic =0;
  int decaytype = -1; 
  //bool WHadronic = false;
  //bool WZEvent =false;
  int NumberW = 0;
  int NumberZ = 0;
  NumberHiggs=0;

  // Custom variables to keep track of the number of different types of events internally (not ROOT) so they can be printed.
  int NumLeptonic = 0;
  int NumSemileptonic = 0;
  int NumHadronic = 0;

  //std::vector<int> decayPDG;
  //std::vector<int> leptonPDG;

  if( col != NULL )
    {
      int nMCP = col->getNumberOfElements()  ;

      
      if(mydebug){
	std::cout << "nMCP = " << nMCP << std::endl;
      }
      
      for(int i=0; i< nMCP ; i++)
	{
	  MCParticle* mcp = dynamic_cast<MCParticle*>( col->getElementAt( i ) ) ;
	  
	  if(mydebug){
	    std::cout << "mcp = " << mcp << std::endl;
	  }

	  if(mydebug){
	    std::cout << "Particle number " << i << " is a " << mcp->getPDG() << std::endl;
	    //std::cout << "Particle four vector " << i << " is a " << mcp->ParticleVec() << std::endl;
	  }
	  if(mcp->getPDG()== 25) {NumberHiggs++;}
	  if(abs(mcp->getPDG())== 24) {NumberW++;}
	  if(abs(mcp->getPDG())== 23) {NumberZ++;}
	  
	  
	  if(mydebug){
	    std::cout << "getParents()[0] size = " << mcp->getParents().size() << std::endl;
	    
	    //if(mcp->getParents()[0]->getParents()[0]->getParents().size() != 0){
	    //std::cout << "Not equal to zero" << std::endl;
	    //}
	    // else{
	    //   std::cout<< "getParents is null" << std::endl;
	    // }
	  }
	  
	  //We're not looking for WZ events in this case, so this is commented out.

//	  if((abs(mcp->getPDG())==14 || abs(mcp->getPDG())==16) // if the hvv neutrinos aren't electron neutrinos
//	     && mcp->getParents()[0]->getParents()[0]->getParents().size()==0)
//	    {
//	      WZEvent=true;
//	      if(mydebug){
//		std::cout << "mcp->getParents()[0] " << mcp->getParents()[0] << std::endl;
//		std::cout<<"WZ event spotted\n";
//	      }
//	    }
	  
	  if(mcp->getParents().size()>0 
	     && abs(mcp->getParents()[0]->getPDG())==24) // If the  particle in question decayed from a W
	    {
	      if( abs(mcp->getPDG())==11 || abs(mcp->getPDG())==12 || abs(mcp->getPDG())==13 || abs(mcp->getPDG())==14 || abs(mcp->getPDG())==15 || abs(mcp->getPDG())==16) // If it's a lepton or neutrino
		{
		  WLeptonic++;
		}
	    }
	}
      
      if(WLeptonic==2) // semileptonic -> if there are ONLY two leptons, both of which are daughters of W's, it can't be anything else.
	{
	  decaytype=1;
	  _nParticleSemiLeptonic->Fill(nMCP);
	}

      else if(WLeptonic==4) // fully leptonic -> if there are four leptons that are all daughters of W's, it can't be anything else. 
	{
	  decaytype=2;
	  _nParticleLeptonic->Fill(nMCP);
	}

      else if(WLeptonic==0) // fully hadronic -> if there are no leptons, all the W decay products must be quarks
	{
	  decaytype=3;
	  _nParticleHadronic->Fill(nMCP);
	}

      else // other -> since we only have three possible types of decays for the ttV process, nothing else needs to be defined and this is more for error catching
	{
	  decaytype=0;
	  _nParticleUnclassified->Fill(nMCP);
	}

      // Old event definitions and sorting are below, just in case.

//      if(NumberW==2 && WLeptonic==2 && WHadronic==true && WZEvent==false) //semileptonic ->two W's, one W decaying to lepton and neutrino, one W decaying to anything else. Both W's decay from Higgs decay
//	{
//	  decaytype=1;
//	  _nParticleSemiLeptonic->Fill(nMCP);
//	} 
      
//      else if(NumberW==2 && WLeptonic==4 && WHadronic==false && WZEvent==false) //fully leptonic-> two W's decaying to lepton and neutrino, both W's are from Higgs decay
//	{
//	  decaytype=2;
//	  _nParticleLeptonic->Fill(nMCP);
//	}
      
//      else if(NumberW==2 && WLeptonic==0 && WHadronic==true && WZEvent==false) //hadronic -> two W's decaying to products that aren't a lepton or a neutrino, both W's are from Higgs decay
//	{
//	  decaytype=3;
//	  _nParticleHadronic->Fill(nMCP);
//	} 

//      else if(WZEvent==true)
//	{
//	  decaytype=4;
//	}

//      else //UnclassifiedEvent
//	{
//	  decaytype=0;
//
//	  _nParticleUnclassified->Fill(nMCP);
//	} 

    }
	   
  std::cout<<"Number of leptonic decays "<<NumLeptonic<<std::endl;
  std::cout<<"Number of semileptonic decays "<<NumSemileptonic<<std::endl;
  std::cout<<"Number of hadronic decays "<<NumHadronic<<std::endl;

  if(debug)
    {
      std::cout<<"Number of Higgs "<<NumberHiggs<<std::endl;
      std::cout<<"Number of Z "<<NumberZ<<std::endl;
      std::cout<<"Number of leptonic decays "<<NumLeptonic<<std::endl;
      std::cout<<"Number of semileptonic decays "<<NumSemileptonic<<std::endl;
      std::cout<<"Number of hadronic decays "<<NumHadronic<<std::endl;
    }

  //std::cout<<"Number of W  particles "<< NumberW<<std::endl;
  //std::cout<<"WLeptonic "<< WLeptonic<<std::endl;
  //std::cout<<"WHadronic "<< WHadronic<<std::endl;
  //std::cout<<"Decaytype "<<decaytype<<std::endl;

  //fill histograms 
  _ndecaytype->Fill(decaytype);

  std::cout << "   processing event: " << evt->getEventNumber() 
	    << "   in run:  " << _nRun << std::endl ;

  _nleptons->Fill(WLeptonic);

  if(_choice==5)
    {
      npassed++;
      if(debug) {std::cout<<"Processor set to pass all events....passing event";}
    }

  else if(decaytype!=_choice)
    {
	 
      nskipped++;
      if(debug) {std::cout<<"Does not match requested decay chain....skipping event"<<_nEvt<<std::endl;}
      _nEvt++;
      throw SkipEventException( this );
    }

  else 
    {
      npassed++;
	  
    }

  _nEvt ++ ;
}



void SignalSeparator::check( LCEvent * evt ) { 
  // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void SignalSeparator::end(){ 

  std::cout << "SignalSeparator::end()  " << name() 
    	    << " processed " << _nEvt << " events in " << _nRun << " runs "
    	    << std::endl ;
  std::cout<< "Events passed: "<< npassed<< "\n"<<"Events rejected: " << nskipped <<std::endl;
  std::cout<<"Number of Higgs bosons: "<< NumberHiggs <<std::endl;
  std::cout<<"Number of Z bosons: "<< NumberHiggs <<std::endl;
  if(_choice==5) // These aren't declared in this scope (in the end() functon), so it throws errors.
    {
      // std::cout<<"Number of leptonic decays: "<<NumLeptonic<<std::endl;
      //std::cout<<"Number of semileptonic decays: "<<NumSemileptonic<<std::endl;
      //std::cout<<"Number of hadronic decays: "<<NumHadronic<<std::endl;
    }

}






