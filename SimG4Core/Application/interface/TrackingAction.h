#ifndef SimG4Core_TrackingAction_H
#define SimG4Core_TrackingAction_H

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "SimG4Core/Notification/interface/SimActivityRegistry.h"

#include "G4UserTrackingAction.hh"

//@@@celeritas 
#include "accel/LocalTransporter.hh"
#include "accel/SharedParams.hh"
//@@@celeritas 

class EventAction;
class TrackWithHistory;
class BeginOfTrack;
class EndOfTrack;
class CMSSteppingVerbose;
class TrackInformation;

class TrackingAction : public G4UserTrackingAction {
  //!@{
  //! \name Type aliases
  using SPConstParams = std::shared_ptr<const celeritas::SharedParams>;
  using SPTransporter = std::shared_ptr<celeritas::LocalTransporter>;
  //!@}
public:
  explicit TrackingAction(EventAction* ea, const edm::ParameterSet& ps, CMSSteppingVerbose*,
			  SPConstParams params, SPTransporter transporter);
  ~TrackingAction() override;

  void PreUserTrackingAction(const G4Track* aTrack) override;
  void PostUserTrackingAction(const G4Track* aTrack) override;

  inline TrackWithHistory* currentTrackWithHistory() { return currentTrack_; }
  inline const G4Track* geant4Track() const { return g4Track_; }
  inline G4TrackingManager* getTrackManager() { return fpTrackingManager; }

  SimActivityRegistry::BeginOfTrackSignal m_beginOfTrackSignal;
  SimActivityRegistry::EndOfTrackSignal m_endOfTrackSignal;

private:
  EventAction* eventAction_;
  CMSSteppingVerbose* steppingVerbose_;
  const G4Track* g4Track_ = nullptr;
  TrackInformation* trkInfo_ = nullptr;
  TrackWithHistory* currentTrack_ = nullptr;
  bool checkTrack_;
  bool doFineCalo_;
  bool saveCaloBoundaryInformation_;
  double eMinFine_;

  //@@@celeritas 
  bool is_EM_;
  SPConstParams celeritasParams_;
  SPTransporter celeritasTransporter_;
  //@@@celeritas 
};

#endif
