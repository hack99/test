// Copyright 2010 Christophe Henry
// henry UNDERSCORE christophe AT hotmail DOT com
// This is an extended version of the state machine available in the boost::mpl library
// Distributed under the same license as the original.
// Copyright for the original version:
// Copyright 2005 David Abrahams and Aleksey Gurtovoy. Distributed
// under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
// back-end
#include <boost/msm/back/state_machine.hpp>
//front-end
#include <boost/msm/front/state_machine_def.hpp>

#include <boost/stm/transaction.hpp>

namespace msm = boost::msm;
namespace mpl = boost::mpl;

namespace
{
  // events
  struct play {};
  struct end_pause {};
  struct stop {};
  struct pause {};
  struct open_close {};

  // A "complicated" event type that carries some data.
  enum DiskTypeEnum
  {
    DISK_CD=0,
    DISK_DVD=1
  };
  struct cd_detected
  {
    cd_detected(std::string name, DiskTypeEnum diskType)
        : name(name),
        disc_type(diskType)
    {}

    std::string name;
    DiskTypeEnum disc_type;
  };

	class handler
	{
	public:
		void handle(const char* event)
		{
			std::cout << "handler::handle(" << event << ")" << std::endl;
		} 
	};

  // front-end: define the FSM structure
  struct player_ : public msm::front::state_machine_def<player_>
  {
    // The list of FSM states
    struct Empty : public msm::front::state<>
    {
      // every (optional) entry/exit methods get the event passed.
      template <class Event,class FSM>
      void on_entry(Event const&,FSM& ) {std::cout << "entering: Empty" << std::endl;}
      template <class Event,class FSM>
      void on_exit(Event const&,FSM& ) {std::cout << "leaving: Empty" << std::endl;}
    };
    struct Open : public msm::front::state<>
    {
      template <class Event,class FSM>
      void on_entry(Event const& ,FSM&) {std::cout << "entering: Open" << std::endl;}
      template <class Event,class FSM>
      void on_exit(Event const&,FSM& ) {std::cout << "leaving: Open" << std::endl;}
    };

    // sm_ptr still supported but deprecated as functors are a much better way to do the same thing
    struct Stopped : public msm::front::state<msm::front::default_base_state,msm::front::sm_ptr>
    {
      template <class Event,class FSM>
      void on_entry(Event const& ,FSM&) {std::cout << "entering: Stopped" << std::endl;}
      template <class Event,class FSM>
      void on_exit(Event const&,FSM& ) {std::cout << "leaving: Stopped" << std::endl;}
      void set_sm_ptr(player_* pl)
      {
        m_player=pl;
      }
      player_* m_player;
    };

    struct Playing : public msm::front::state<>
    {
      template <class Event,class FSM>
      void on_entry(Event const&,FSM& ) {std::cout << "entering: Playing" << std::endl;}
      template <class Event,class FSM>
      void on_exit(Event const&,FSM& ) {std::cout << "leaving: Playing" << std::endl;}
    };

    // state not defining any entry or exit
    struct Paused : public msm::front::state<>
      {}
    ;

    // the initial state of the player SM. Must be defined
    typedef Empty initial_state;

    // transition actions
    void start_playback(play const&)       { std::cout << "player::start_playback\n"; handler_->handle("play"); }
    void open_drawer(open_close const&)    { std::cout << "player::open_drawer\n";  handler_->handle("open_close"); }
    void close_drawer(open_close const&)   { std::cout << "player::close_drawer\n";  handler_->handle("open_close"); }
    void store_cd_info(cd_detected const&) { std::cout << "player::store_cd_info\n"; handler_->handle("cd_detected"); }
    void stop_playback(stop const&)        { std::cout << "player::stop_playback\n"; handler_->handle("stop"); }
    void pause_playback(pause const&)      { std::cout << "player::pause_playback\n"; handler_->handle("pause"); }
    void resume_playback(end_pause const&)      { std::cout << "player::resume_playback\n"; handler_->handle("end_pause"); }
    void stop_and_open(open_close const&)  { std::cout << "player::stop_and_open\n"; handler_->handle("open_close"); }
    void stopped_again(stop const&){std::cout << "player::stopped_again\n"; handler_->handle("stop");  }
    // guard conditions
    bool good_disk_format(cd_detected const& evt)
    {
      // to test a guard condition, let's say we understand only CDs, not DVD
      if (evt.disc_type != DISK_CD)
      {
        std::cout << "wrong disk, sorry" << std::endl;
        return false;
      }
      return true;
    }
    // used to show a transition conflict. This guard will simply deactivate one transition and thus
    // solve the conflict
    bool auto_start(cd_detected const&)
    {
      return false;
    }

    typedef player_ p; // makes transition table cleaner

    // Transition table for player
    struct transition_table : mpl::vector<
          //    Start     Event         Next      Action               Guard
          //  +---------+-------------+---------+---------------------+----------------------+
          a_row < Stopped , play        , Playing , &p::start_playback                         >,
          a_row < Stopped , open_close  , Open    , &p::open_drawer                            >,
          _row < Stopped , stop        , Stopped                                              >,
          //  +---------+-------------+---------+---------------------+----------------------+
          a_row < Open    , open_close  , Empty   , &p::close_drawer                           >,
          //  +---------+-------------+---------+---------------------+----------------------+
          a_row < Empty   , open_close  , Open    , &p::open_drawer                            >,
          row < Empty   , cd_detected , Stopped , &p::store_cd_info   ,&p::good_disk_format  >,
          row < Empty   , cd_detected , Playing , &p::store_cd_info   ,&p::auto_start        >,
          //  +---------+-------------+---------+---------------------+----------------------+
          a_row < Playing , stop        , Stopped , &p::stop_playback                          >,
          a_row < Playing , pause       , Paused  , &p::pause_playback                         >,
          a_row < Playing , open_close  , Open    , &p::stop_and_open                          >,
          //  +---------+-------------+---------+---------------------+----------------------+
          a_row < Paused  , end_pause   , Playing , &p::resume_playback                        >,
          a_row < Paused  , stop        , Stopped , &p::stop_playback                          >,
          a_row < Paused  , open_close  , Open    , &p::stop_and_open                          >
          //  +---------+-------------+---------+---------------------+----------------------+
      > {};
    // Replaces the default no-transition response.
    template <class FSM,class Event>
    void no_transition(Event const& e, FSM&,int state)
    {
      std::cout << "no transition from state " << state
      << " on event " << typeid(e).name() << std::endl;
    }

	player_(handler *h) : handler_(h)
	{}
	protected:
		handler *handler_;
  };
  // Pick a back-end
  typedef msm::back::state_machine<player_> player;

  //
  // Testing utilities.
  //
  static char const* const state_names[] = { "Stopped", "Open", "Empty", "Playing", "Paused" };
  void pstate(player const& p)
  {
    std::cout << " -> " << state_names[p.current_state()[0]] << std::endl;
  }

using namespace boost::stm;

class player_object : public transaction_object<player_object>
{
public:
	player_object() : p_(&handler_)
	{
	}

	void start()
	{
		transaction *tx = current_transaction();
		tx->write_ptr(this)->p_.start();
	}

	template <class T>
	void process_event(T event)
	{
		transaction *tx = current_transaction();
		tx->write_ptr(this)->p_.process_event(event);	
	}

	player& get_player() 
	{
		transaction *tx = current_transaction(); 
		return tx->write_ptr(this)->p_; 
	}
protected:
	handler handler_;
	player p_;
};

void trans_test()
{
	player_object p;
	try_atomic (t)
	{
		p.start();
		// go to Open, call on_exit on Empty, then action, then on_entry on Open
		p.process_event(open_close()); pstate(p.get_player());
		p.process_event(open_close()); pstate(p.get_player());
		// will be rejected, wrong disk type
		p.process_event(
		cd_detected("louie, louie",DISK_DVD)); pstate(p.get_player());
		p.process_event(
		cd_detected("louie, louie",DISK_CD)); pstate(p.get_player());
		p.process_event(play());
		
		// at this point, Play is active
		p.process_event(pause()); pstate(p.get_player());
		// go back to Playing
		p.process_event(end_pause());  pstate(p.get_player());
		p.process_event(pause()); pstate(p.get_player());
		p.process_event(stop());  pstate(p.get_player()); 
		// event leading to the same state
		// no action method called as it is not present in the transition table
		p.process_event(stop());  pstate(p.get_player());
	
	} end_atom

}

}



int main()
{
	boost::stm::transaction::initialize();
	boost::stm::transaction::initialize_thread();
  trans_test();
  return 0;
}
