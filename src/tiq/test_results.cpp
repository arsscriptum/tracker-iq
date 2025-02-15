
//==============================================================================
//
// test_results.cpp :test_results container
//
//==============================================================================
//  Copyright (C) Guilaume Plante 2020 <cybercastor@icloud.com>
//==============================================================================

#include "stdafx.h"
#include "test_results.h"

#include "config.h"

void TrackerTest::to_json(std::ostringstream& stream) {
	stream << "{"
		<< "\"unique_id\": " << _unique_id << ", "
		<< "\"is_valid\": " << (_is_valid ? "true" : "false") << ", "
		<< "\"num_peers\": " << _num_peers << ", "
		<< "\"response_time\": " << std::fixed << std::setprecision(4) << get_response_time() << ", "
		<< "\"url\": \"" << _url << "\", "
		<< "\"last_tested\": \"" << _test_time << "\", "
		<< "\"rank_score\": " << std::fixed << std::setprecision(4) << _ranking
		<< "}" <<  std::endl;
}

void TrackerTest::dump_all(std::ostringstream& stream)  {
	stream << "TrackerTest Dump:\n"
		<< "Unique ID: " << _unique_id << "\n"
		<< "Is Valid: " << (_is_valid ? "true" : "false") << "\n"
		<< "Number of Peers: " << _num_peers << "\n"
		<< "Response Time: " << std::fixed << std::setprecision(4) << _response_time << "\n"
		<< "URL: " << _url << "\n"
		<< "Test Time: " << _test_time << "\n"
		<< "rank score: " << std::fixed << std::setprecision(4) << _ranking << "\n";
}

void TrackerTest::dump_min(std::ostringstream& stream) 
{
	stream << "[" << _unique_id << "] [" << (_is_valid ? "valid]" : "invalid]") << " p: " << _num_peers << " ping: " << std::fixed << std::setprecision(4) << _response_time << " rank_score " << std::fixed << std::setprecision(4) << _ranking;
}

void TrackerTest::set_ranking(float val) {
	LOG_WARNING("RANKING", "SET VALUE TO %.5f", val);
	this->_ranking = val;
	LOG_WARNING("RANKING", "CONFIRN VALUE TO %.5f", this->_ranking);
}

void TrackerTestResults::to_json() {

	bool first = true;

	std::cout << "[" << std::endl << std::flush;
	
		for (std::vector<TrackerTest>::iterator it = results.begin(); it != results.end(); ++it) {

			if (!first) {
				std::cout << "," << std::endl;
			}
			else {
				first = false;
			}

			TrackerTest* res = &(*it);
			float r = res->get_ranking();
			std::ostringstream dbg_output;
			res->to_json(dbg_output);
			std::cout << std::endl << "[DEBUG] to_json " << dbg_output.str() << std::endl << std::endl;

			std::cout  << "{"
				<< "\"unique_id\": " << res->get_unique_id() << ", "
				<< "\"is_valid\": " << (res->get_is_valid() ? "true" : "false") << ", "
				<< "\"num_peers\": " << res->get_num_peers() << ", "
				<< "\"response_time\": " << std::fixed << std::setprecision(4) << res->get_response_time() << ", "
				<< "\"url\": \"" << res->get_url() << "\", "
				<< "\"last_tested\": \"" << res->get_test_time() << "\", "
				<< "\"rank_score\": " << std::fixed << std::setprecision(4) << res->get_ranking()
				<< "}" << std::endl;
		}

	std::cout << std::endl << "]" << std::endl << std::flush;
	


}

int TrackerTestResults::sanitize_results()
{
	int num = 0;
	for (auto it = results.begin(); it != results.end(); ) {
		if (!it->get_is_valid()) {
			num++;
			it = results.erase(it); // Removes the element and returns the next valid iterator
		}
		else {
			++it; // Only increment if we didn't erase
		}
	}
	return num;
}


void TrackerTestResults::add_result(const TrackerTest& result)
{
	results.push_back(result);
}

void TrackerTestResults::addResult(unsigned int id, bool valid, unsigned int peers, double time, const std::string& tracker_url, const std::string& time_str) 
{
	TrackerTest* ptr = &results.emplace_back(id, valid, peers, time, tracker_url, time_str);
	if (valid && peers) {
		if (peers < _min_peers) { _min_peers = peers; }
		if (peers > _max_peers) { _max_peers = peers; }
		if (time < _min_response_time) { _min_response_time = time; }
		if (time > _max_response_time) { _max_response_time = time; }
	}
	std::ostringstream dbg_output;
	ptr->dump_min(dbg_output);
	LOG_TRACE("TrackerTestResults::addResult", "add %d, %s", Count(), dbg_output.str().c_str());
}

unsigned int TrackerTestResults::Count() 
{
	return (unsigned int)results.size();
}

bool TrackerTestResults::empty() 
{
	return results.empty();
}

std::vector<TrackerTest>& TrackerTestResults::get_results() 
{
	return results;
}

const std::vector<TrackerTest>& TrackerTestResults::get_results_const() const 
{
	return results;
}
