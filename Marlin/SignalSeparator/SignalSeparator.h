#ifndef SignalSeparator_h
#define SignalSeparator_h 1

#include "marlin/Processor.h"
#include "lcio.h"
#include <string>
#include <map>
#include <EVENT/ReconstructedParticle.h>
#include <EVENT/MCParticle.h>
#include <EVENT/LCRelation.h>
#include <UTIL/LCRelationNavigator.h>
#include "TH1F.h"

using namespace lcio ;
using namespace marlin ;


/**  Example processor for marlin.
 * 
 *  If compiled with MARLIN_USE_AIDA 
 *  it creates a histogram (cloud) of the MCParticle energies.
 * 
 *  <h4>Input - Prerequisites</h4>
 *  Needs the collection of MCParticles.
 *
 *  <h4>Output</h4> 
 *  A histogram.
 * 
 * @param CollectionName Name of the MCParticle collection
 * 
 * @author F. Gaede, DESY
 * @version $Id: SignalSeparator.h,v 1.4 2005-10-11 12:57:39 gaede Exp $ 
 */

class SignalSeparator : public Processor {
  
 public:
  
  virtual Processor*  newProcessor() { return new SignalSeparator ; }
  
  
  SignalSeparator() ;
  
  /** Called at the begin of the job before anything is read.
   * Use to initialize the processor, e.g. book histograms.
   */
  virtual void init() ;
  
  /** Called for every run.
   */
  virtual void processRunHeader( LCRunHeader* run ) ;
  
  /** Called for every event - the working horse.
   */
  virtual void processEvent( LCEvent * evt ) ; 
  
  
  virtual void check( LCEvent * evt ) ; 
  
  
  /** Called after data processing for clean up.
   */
  virtual void end() ;
  
  int _choice;  

 protected:

  /** Input collection name.
   */
  std::string _colMCTL ;
  std::string _colMCP ;
  std::string _colJet ;

  int _nRun ;
  int _nEvt ;
  int _nZ ;
  int _nZQuarks ;
  int _nHiggs ;
  int _nHiggsQuarks ;

  int _higgscounter;
  int _Zcounter;

  int nskipped;
  int npassed;
  int NumberHiggs;
  int NumLeptons;
  int NumSemileptonic;
  int NumHadronic;

  // Histograms
  TH1F* _nleptons;
  TH1F* _nWDecays;
  TH1F* _ndecayproducts;
  TH1F* _ndecayPDG;
  TH1F* _nleptonPDG;
  TH1F* _ndecaytype;
  TH1F* _nParticleLeptonic;
  TH1F* _nParticleSemiLeptonic;
  TH1F* _nParticleHadronic;
  TH1F* _nParticleUnclassified;
  TH1F* _ptMCPLeptons;
  TH1F* _ptRPLeptons;
  TH1F* _ptMCPLeptons_vis;
  TH1F* _ptMCPQuarks;
  TH1F* _massMCPHiggs;
  TH1F* _massMCPZboson;
  TH1F* _massMCPV;
  TH1F* _massMCPbottom;
  TH1F* _massRPV;
  TH1F* _ptRPPDG;
  TH1F* _ptMCPDG;
  TH1F* _PDGMCP;
  TH1F* _ptRPType;
  TH1F* _angle;
  TH1F* _RPlikelyhood;
  TH1F* _ptMCPtl;
  TH1F* _ptRPtl;
  TH1F* _generated_MCPtl_pt;
  TH1F* _generated_RPtl_pt;
  TH1F* _bestGuess;
  TH1F* _ptBestGuess;
  TH1F* _angleBestGuess;
  TH1F* _xangle;
  TH1F* _yangle;
  TH1F* _zangle;
  TH1F* _ptDiffBestGuess;
  TH1F* _ptDiffBadGuess;


  TH1F* _jetMinP;
  TH1F* _jetsBestGuessAngleHiggs;
  TH1F* _jetsBestGuessAngleZ;
  TH1F* _jetsbestGuesspt;
  TH1F* _jetOptH; 
  TH1F* _jetOptZ;
  TH1F* _jetPairSep;
  TH1F* _jetsAngSep;
  TH1F* _jetsAngSep1 ;
  TH1F* _jetsAngSep2; 


 // Histograms for jet transverse momenta
  float jetPT[8];
  float jetE[8];
  float jetPTn[8];
  float jetEn[8];
  float jetPx[8];
  float jetPy[8];
  float jetPz[8];
  

  //float jetPairMomentum[2];

  TH1I* _jet_distro;
  TH1F* _jetPThisto1;
  TH1F* _jetPThisto2;
  TH1F* _jetPThisto3;
  TH1F* _jetMatchedPThisto;
  //  TH1F* _jetInvMass;
  TH1F* _jetInvMassZ;
  TH1F* _jetInvMassHiggs;
  TH1F* _jetInvMassmcptl;
  TH1F* _jetInvMassZmcptl;
  TH1F* _jetInvMassHmcptl;
  TH1F* _leptonInvmass;

  TH1F* _jetsZAngSep1;
  TH1F* _jetsZAngSep2;
  TH1F* _jetsHAngSep1;
  TH1F* _jetsHAngSep2;

  // Variables use for jet momentum matching
  float BestGuessJetMomentum;
  int BestGuessJetIndex;
  float testjetPMag;
  float thisjetMag;
  float bestGuessInvMassZ;
  float bestGuessInvMassHiggs;



  //Variables used for lepton Invaraint Mass Calculation
  int BestGuessParent;
  float bestGuessLepInvMass;
  float lepInvMass;
  float xs;
  float ys;
  float zs;
  float Etot;
  float singLepInvMass;





 
  //Variables use for the jet angle matching using inv mass
  float bestGuessZAngle1;
  float bestGuessZAngle2;
  float bestGuessHAngle1;
  float bestGuessHAngle2;

  float bestGuessAngle1;
  float bestGuessAngle2;
  float DotProductmcpt1;
  float DotProductmcpt2;
  float jet1Mag;
  float jet2Mag;
  float angle1;
  float angle2;
  int jet1IndZ;
  int jet2IndZ;
  int jet1IndHiggs;
  int jet2IndHiggs;


  // Stuff

  float DotProduct;
  float DotProductmcpt;
  float ThisAnglemcpt;
  float BestGuesspt;
  float BestGuessAnglemcpt;
  float BestGuessEnergy;
  float momSumMagSq;
  float momSumMagSqH;
  float momSumMagSqZ;
  float adjInvMass;

  float adjInvMassZ;
  float adjInvMassHiggs;


  
  float rptlPMag;
  float mcpPMag;
  float mcptPMag;
  float mcptPMagZ;
  float mcptPMagH;
  float invmass;


  float BestGuessAngle;
  float BestGuessMomentum;
  int BestGuessPDG;
  int BestGuessCharge;

  int NoMatchParticles;

  float ThisAngle;
  float ThisMomentum;

  float xDotProduct;
  float yDotProduct;
  float zDotProduct;

  MCParticleVec* HiggsDaughterList;
  MCParticle* BestGuessmcp;

  int BottomHiggsDecays;
  int NonBottomHiggsDecays;

  //AXIS VECTORS//
  float xAxis[3];
  float yAxis[3];
  float zAxis[3];
} ;

#endif



