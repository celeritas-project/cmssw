#include "SimG4Core/Application/interface/EventAction.h"
#include "SimG4Core/Application/interface/SimRunInterface.h"
#include "SimG4Core/Notification/interface/G4SimVertex.h"
#include "SimG4Core/Notification/interface/G4SimTrack.h"
#include "SimG4Core/Notification/interface/BeginOfEvent.h"
#include "SimG4Core/Notification/interface/EndOfEvent.h"
#include "SimG4Core/Notification/interface/CMSSteppingVerbose.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "Randomize.hh"

//@@@celeritas
#include "corecel/Macros.hh"
#include "accel/ExceptionConverter.hh"
//@@@celeritas

EventAction::EventAction(const edm::ParameterSet& p,
                         SimRunInterface* rm,
                         SimTrackManager* iManager,
                         CMSSteppingVerbose* sv,
                         SPTransporter transporter)
    : m_runInterface(rm),
      m_trackManager(iManager),
      m_SteppingVerbose(sv),
      m_stopFile(p.getParameter<std::string>("StopFile")),
      m_printRandom(p.getParameter<bool>("PrintRandomSeed")),
      m_debug(p.getUntrackedParameter<bool>("debug", false)), 
      m_celeritasTransporter(std::move(transporter)) 
{
  m_trackManager->setCollapsePrimaryVertices(p.getParameter<bool>("CollapsePrimaryVertices"));
}

EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event* anEvent) {
  m_trackManager->reset();

  BeginOfEvent e(anEvent);
  m_beginOfEventSignal(&e);

  if (m_printRandom) {
    edm::LogVerbatim("SimG4CoreApplication")
        << "BeginOfEvent " << anEvent->GetEventID() << " Random number: " << G4UniformRand();
  }

  if (nullptr != m_SteppingVerbose) {
    m_SteppingVerbose->BeginOfEvent(anEvent);
  }

  //@@@celeritas
  // Set event ID in local transporter
  celeritas::ExceptionConverter call_g4exception{"celer0002"};
  CELER_TRY_HANDLE(m_celeritasTransporter->SetEventId(anEvent->GetEventID()),
                   call_g4exception);
  //@@@celeritas
}

void EventAction::EndOfEventAction(const G4Event* anEvent) {
  if (m_printRandom) {
    edm::LogVerbatim("SimG4CoreApplication")
        << " EndOfEvent " << anEvent->GetEventID() << " Random number: " << G4UniformRand();
  }
  if (!m_stopFile.empty() && std::ifstream(m_stopFile.c_str())) {
    edm::LogWarning("SimG4CoreApplication")
        << "EndOfEventAction: termination signal received at event " << anEvent->GetEventID();
    // soft abort run
    m_runInterface->abortRun(true);
  }
  if (anEvent->GetNumberOfPrimaryVertex() == 0) {
    edm::LogWarning("SimG4CoreApplication") << "EndOfEventAction: event " << anEvent->GetEventID()
                                            << " must have failed (no G4PrimaryVertices found) and will be skipped ";
    return;
  }

  m_trackManager->storeTracks(m_runInterface->simEvent());

  // dispatch now end of event, and only then delete tracks...
  EndOfEvent e(anEvent);
  m_endOfEventSignal(&e);

  m_trackManager->deleteTracks();
  m_trackManager->cleanTkCaloStateInfoMap();

  //@@@celeritas
  // Transport any tracks left in the buffer
  celeritas::ExceptionConverter call_g4exception{"celer0004"};
  CELER_TRY_HANDLE(m_celeritasTransporter->Flush(), call_g4exception);
  //@@@celeritas
}

void EventAction::addTkCaloStateInfo(uint32_t t, const std::pair<math::XYZVectorD, math::XYZTLorentzVectorD>& p) {
  m_trackManager->addTkCaloStateInfo(t, p);
}

void EventAction::abortEvent() { m_runInterface->abortEvent(); }
