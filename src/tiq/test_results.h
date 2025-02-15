
//==============================================================================
//
// test_results.h :test_results container
//
//==============================================================================
//  Copyright (C) Guilaume Plante 2020 <cybercastor@icloud.com>
//==============================================================================


#ifndef __TEST_RESULTS_H__
#define __TEST_RESULTS_H__


#include <iostream>
#include <fstream>
#include "httplib.h"
#include <regex>
#include <vector>
#include "log.h"
#include <string>

class TrackerTest {
private:
    unsigned int _unique_id;
    bool _is_valid;
    unsigned int _num_peers;
    double _response_time;
    std::string _url;
    std::string _test_time;
    float _ranking;     // Computed ranking score
public:
    TrackerTest() : _unique_id(-1), _is_valid(false), _num_peers(0), _response_time(0.f), _url(""), _test_time(""), _ranking(0.f) {}

    TrackerTest(unsigned int id, bool valid, unsigned int peers, double time, const std::string& tracker_url, const std::string& time_str)
        : _unique_id(id), _is_valid(valid), _num_peers(peers), _response_time(time), _url(tracker_url), _test_time(time_str), _ranking(0.f) {}

    unsigned int get_unique_id() { return _unique_id; }
    bool get_is_valid() const { return _is_valid; }
    unsigned int get_num_peers() { return _num_peers; }
    double get_response_time() { return _response_time; }
    float get_ranking() { return this->_ranking; }
    double get_response_time_ms() { return _response_time * 1000; }
    std::string get_url() { return _url; }
    std::string get_test_time() { return _test_time; }

    void set_unique_id(unsigned int id) { _unique_id = id; }
    void set_is_valid(bool valid) { _is_valid = valid; }
    void set_num_peers(unsigned int peers) { _num_peers = peers; }
    void set_ranking(float s);
    void set_response_time(double time) { _response_time = time; }
    void set_url(const std::string& tracker_url) { _url = tracker_url; }
    void set_test_time(const std::string& time_str) { _test_time = time_str; }
    void dump_all(std::ostringstream& stream);
    void dump_min(std::ostringstream& stream);
    void to_json(std::ostringstream& stream) ;
};

class TrackerTestResults {
private:
    std::vector<TrackerTest> results;
    unsigned int _min_peers;
    unsigned int _max_peers;
    double _min_response_time;
    double _max_response_time;
public:
    TrackerTestResults::TrackerTestResults() : _min_peers(0), _max_peers(0), _min_response_time(0.f), _max_response_time(0.f) {}
    TrackerTestResults::~TrackerTestResults() {};

    // accessors
    unsigned int get_max_peers() const { return _max_peers; }
    unsigned int get_min_peers() const { return _min_peers; }
    double get_min_response_time() const { return _min_response_time; }
    double get_max_response_time() const { return _max_response_time; }

    // cleaning upda the invalid results
    int sanitize_results();

    void add_result(const TrackerTest& result); 
    void addResult(unsigned int id, bool valid, unsigned int peers, double time, const std::string& tracker_url, const std::string& time_str);

    unsigned int Count();
    bool empty();

    std::vector<TrackerTest>& get_results();
    const std::vector<TrackerTest>& get_results_const() const;
    void to_json();
};


#endif // __TEST_RESULTS_H__