#ifndef __PCTSP_EVENT_HANDLERS__
#define __PCTSP_EVENT_HANDLERS__

#include "graph.hh"
#include "stats.hh"

const std::string NODE_EVENTHDLR_NAME = "pctsp_node_handler";

NodeStats newStatsForNode(SCIP* scip, SCIP_NODE* node);

unsigned int currentNodeId(SCIP* scip);

/** Event handler for nodes of the branch and bound tree */
class NodeEventhdlr : public scip::ObjEventhdlr
{
   std::vector<NodeStats> node_stats_;
public:
   /** default constructor */
   NodeEventhdlr(
      SCIP* scip
      )
      : ObjEventhdlr(scip, NODE_EVENTHDLR_NAME.c_str() ,"event handler for nodes in branch and bound tree for PCTSP")
   {
      node_stats_ = {};
   }

   /** Get the node statistics */
   std::vector<NodeStats> getNodeStatsVector();

   NodeStats getNodeStats(SCIP_NODE* node);

   NodeStats getNodeStats(SCIP* scip);

   void addCurrentNode(SCIP* scip);

   void incrementNumSecDisjointTour(SCIP* scip, unsigned int n_cuts);

   void incrementNumSecMaxflowMincut(SCIP* scip, unsigned int n_cuts);

   /** destructor of event handler to free user data (called when SCIP is exiting) */
   virtual SCIP_DECL_EVENTFREE(scip_free);

   /** initialization method of event handler (called after problem was transformed) */
   virtual SCIP_DECL_EVENTINIT(scip_init);

   /** deinitialization method of event handler (called before transformed problem is freed) */
   virtual SCIP_DECL_EVENTEXIT(scip_exit);

   /** solving process initialization method of event handler (called when branch and bound process is about to begin)
    *
    *  This method is called when the presolving was finished and the branch and bound process is about to begin.
    *  The event handler may use this call to initialize its branch and bound specific data.
    *
    */
   virtual SCIP_DECL_EVENTINITSOL(scip_initsol);

   /** solving process deinitialization method of event handler (called before branch and bound process data is freed)
    *
    *  This method is called before the branch and bound process is freed.
    *  The event handler should use this call to clean up its branch and bound data.
    */
   virtual SCIP_DECL_EVENTEXITSOL(scip_exitsol);

   /** frees specific constraint data */
   virtual SCIP_DECL_EVENTDELETE(scip_delete);

   /** execution method of event handler
    *
    *  Processes the event. The method is called every time an event occurs, for which the event handler
    *  is responsible. Event handlers may declare themselves resposible for events by calling the
    *  corresponding SCIPcatch...() method. This method creates an event filter object to point to the
    *  given event handler and event data.
    */
   virtual SCIP_DECL_EVENTEXEC(scip_exec);
};


/** Event handler for keeping track of the lower and upper bounds at timestamps*/
class BoundsEventHandler : public scip::ObjEventhdlr
{
   TimePointUTC _last_timestamp;
   std::vector<Bounds> _bounds_vector;
public:
   /** default constructor */
   BoundsEventHandler(
      SCIP* scip
      )
      : ObjEventhdlr(scip, "pctsp_bound_handler","Record upper and lower bounds")
   {
      using namespace std::chrono;
      _last_timestamp = time_point_cast<SubSeconds>(system_clock::now());
      _bounds_vector = std::vector<Bounds>();
   }

   TimePointUTC getLastTimestamp();

   std::vector<Bounds> getBoundsVector();

   /** destructor of event handler to free user data (called when SCIP is exiting) */
   virtual SCIP_DECL_EVENTFREE(scip_free);

   /** initialization method of event handler (called after problem was transformed) */
   virtual SCIP_DECL_EVENTINIT(scip_init);

   /** deinitialization method of event handler (called before transformed problem is freed) */
   virtual SCIP_DECL_EVENTEXIT(scip_exit);

   /** solving process initialization method of event handler (called when branch and bound process is about to begin)
    *
    *  This method is called when the presolving was finished and the branch and bound process is about to begin.
    *  The event handler may use this call to initialize its branch and bound specific data.
    *
    */
   virtual SCIP_DECL_EVENTINITSOL(scip_initsol);

   /** solving process deinitialization method of event handler (called before branch and bound process data is freed)
    *
    *  This method is called before the branch and bound process is freed.
    *  The event handler should use this call to clean up its branch and bound data.
    */
   virtual SCIP_DECL_EVENTEXITSOL(scip_exitsol);

   /** frees specific constraint data */
   virtual SCIP_DECL_EVENTDELETE(scip_delete);

   /** execution method of event handler
    *
    *  Processes the event. The method is called every time an event occurs, for which the event handler
    *  is responsible. Event handlers may declare themselves resposible for events by calling the
    *  corresponding SCIPcatch...() method. This method creates an event filter object to point to the
    *  given event handler and event data.
    */
   virtual SCIP_DECL_EVENTEXEC(scip_exec);
};

#endif