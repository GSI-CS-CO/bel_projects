/*******************************************************************************************
 *  created : 2022
 *  author  : Michael Reese, GSI-Darmstadt
 *  version : 04-Mar-2022
 *
 * Precise measurement of frequency on B2B-PM-Nodes.
 * The Measurement is based on the same ECA events that the embedded CPU uses to measure the frequency.
 * Because the host system provides floating point numbers and more CPU power a linear regression is
 * possible to get a more precise value of the frequency. Measurement results are published as DIM
 * services.
 *
 * At the moment, the zero-crossing events for the frequency measurement come from the B2B system.
 *
 * TODO: In the future, this tool should generate these events by itself.
 *
 *********************************************************************************************/
#define FREQ_MEASURE_VERSION "00.03.18"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cstdio>
#include <cstdint>
#include <cmath>
#include <cassert>
#include <fstream>

// saftlib includes
#include <TimingReceiver.h>
#include <SoftwareActionSink.h>
#include <SoftwareCondition.h>
#include <SAFTd.h>

// DIM include for service
#include <dis.h>

// B2B definitions
#define B2B_ECADO_B2B_PMEXT        0x800   // this is an event-id internal to the B2B system. command: perform phase measurement (extraction)
                                           // it is used in this program to build an ECA condition that initiates the measurement
#define B2B_ECADO_B2B_PMINJ        0x801   // this is an event-id internal to the B2B system. command: perform phase measurement (injection)

// some group-ids from the B2B system. They are used to build the ECA condition that initiates the measurement.
#define SIS18_B2B_EXTRACT          0x3a0   // GID: SIS18 simple extraction
#define ESR_B2B_EXTRACT            0x3a5   // GID: ESR simple extraction
#define CRYRING_B2B_EXTRACT        0x3aa   // GID: CRYRING simple extraction

struct LinearRegression {

	LinearRegression(const LinearRegression& lr) {
		N          = lr.N;
		x_sum      = lr.x_sum;
		y_sum      = lr.y_sum;
		xy_sum     = lr.xy_sum;
		xx_sum     = lr.xx_sum;
		a          = lr.a;
		b          = lr.b;
		var_a      = lr.var_a;
		var_b      = lr.var_b;
		covar_ab   = lr.covar_ab;
	}
	LinearRegression &operator=(const LinearRegression &lr) {
		N          = lr.N;
		x_sum      = lr.x_sum;
		y_sum      = lr.y_sum;
		xy_sum     = lr.xy_sum;
		xx_sum     = lr.xx_sum;
		var_a      = lr.var_a;
		var_b      = lr.var_b;
		covar_ab   = lr.covar_ab;
		return *this;
	}
	LinearRegression() {
		reset();
	}
	void reset() {
		N          = 0;
		x_sum      = 0.0;
		y_sum      = 0.0;
		xy_sum     = 0.0;
		xx_sum     = 0.0;
	}

	void add_point(double x, double y) {
		x_sum  += x;
		y_sum  += y;
		xx_sum += x*x;
		xy_sum += x*y;
		N += 1;
		calculate();
	}
	void calculate() {
		double D = xx_sum*N - x_sum*x_sum;
		b = (xy_sum*N - x_sum*y_sum) / D;
		a = y_sum/N - b * x_sum/N;
		var_a = xx_sum / D;
		var_b = N / D;
		covar_ab = - x_sum / D;
	}
	double get_a() const {
		return a;
	}
	double get_var_a() const {
		return var_b;
	}
	double get_b() const {
		return b;
	}
	double get_var_b() const {
		return var_b;
	}
	double get_covar_ab() const {
		return covar_ab;
	}
	double get_y(double x) const {
		return a + b * x;
	}
	double get_dy(double x) const {
		return sqrt(var_a + x*x*var_b + 2*x*covar_ab);
	}
	double get_dx(double x) const {
		return get_dy(x)/b;
	}
	double get_x(double y) const {
		return (y - a) / b;
	}
	double get_R(double x, double y) const {
		return y - get_y(x);
	}
	double get_RR(double x, double y) const {
		double R = get_R(x,y);
		return R*R;
	}
	double get_mean_x() const {
		return x_sum / N;
	}
	double get_mean_y() const {
		return y_sum / N;
	}
	// return true if the point (x,y) is close enough to the predicted straight line
	bool evaluate(double x, double y) const {
		double RR = get_RR(x,y);
		double dy = 2*get_dy(x); // limit the acceptance at 4 standard deviations
		return RR < dy*dy;
	}
private:
	int N;
	double x_sum, y_sum, xy_sum, xx_sum;
	double a, b; // y=a+b*x
	double var_a, var_b, covar_ab;
};

std::string error_format(double x, double dx) {
	int precision=log(100/dx)/log(10);
	std::ostringstream out;
	out //<< x << " " << dx << " " << precision << " "
		<< std::fixed << std::setprecision(precision) << x
	    << "(" << (int)round(dx*pow(10,precision)) << ")";
	return out.str();
}



struct FrequencyMeasurement {
	bool valid;
	double freq_Hz, freq_sigma_Hz;
	double freq_slope_kHz_s, freq_slope_sigma_kHz_s;
	int num_bursts;
	int num_points;
	int num_outliers;
	double chi2;
	double red_chi2;

	FrequencyMeasurement(const std::vector<int64_t> &edge_times_ns);
};

// helper struct for the chi^2 calculation at the end of the fit
struct DataPoint {
	int64_t number;
	int64_t time_ns;
	DataPoint(int64_t n, int64_t t) : number(n), time_ns(t) {}
};

// edge_times needs to be sorted
FrequencyMeasurement::FrequencyMeasurement(const std::vector<int64_t> &edge_times_ns) : valid(false) {
	LinearRegression lin_reg_all;
	LinearRegression lin_reg_freq_slope;

	if (edge_times_ns.size() < 3) {
		return;
	}
	assert(std::is_sorted(edge_times_ns.begin(), edge_times_ns.end()));

	// Ignore edge_times_ns[0]. Assume that edge_times_ns[1] and edge_times_ns[2] are valid points
	lin_reg_all.add_point(1, edge_times_ns[1]);
	lin_reg_all.add_point(2, edge_times_ns[2]);
	// if edge_times_ns[0] fits on the line, take it as well
	if (lin_reg_all.evaluate(0,edge_times_ns[0])) {
		lin_reg_all.add_point(0,edge_times_ns[0]);
	}

	// temporary data
	std::vector<DataPoint> points; // needed for chi^2 calculation later
	std::vector<std::vector<int64_t> > bursts(1);


	int64_t previous_number = 0; // keep track of the last period number

	// loop over all data points (make sure that points edge_times_ns[0],[1],[2] are not used again)
	for (int i = 0; i < edge_times_ns.size(); ++i) {
		// calculate the period number based on the time (round to nearest integer)
		int64_t number = round(lin_reg_all.get_x(edge_times_ns[i]));
		// a new burst is detected if the period number jumps by more than 10
		if (i > 2 && (number > previous_number+10)) {
			bursts.push_back(std::vector<int64_t>());
		}
		// add point to the burst
		bursts.back().push_back(edge_times_ns[i]);

		// add the point to the straight line fit only if is in agreement (considering the error of the line fit)
		if (lin_reg_all.evaluate(number, edge_times_ns[i])) {
			points.push_back(DataPoint(number, edge_times_ns[i]));
			if (i > 2) { // points 0,1,2 were are already used
				lin_reg_all.add_point(number, edge_times_ns[i]); // linear regression to all the points
			}
		}
		previous_number = number;
	}

	// calculate chi^2
	for (int i = 0; i < points.size(); ++i) {
		double x = points[i].number;
		double y = points[i].time_ns;
		double sigma = sqrt(1.0/12.0); // = 0.288
		double R = lin_reg_all.get_R(x,y)/sigma; //(y - (a+b*x))/sigma;
		double RR = R*R;
		chi2 += RR;
	}
	red_chi2 = chi2/(points.size()-2);
	num_bursts = bursts.size();
	num_points = points.size();
	num_outliers = edge_times_ns.size()-points.size();

	// linear regression was done with the period time [ns] and period number
	// frequency is the inverse of the period time
	freq_Hz = 1e9/lin_reg_all.get_b();

	// edge mearuements are distributed +- 0.5 ns around the measured value
	// standard deviation of such a distribution is sqrt(1/12)
	freq_sigma_Hz = sqrt(1.0/12.0)* freq_Hz/lin_reg_all.get_b() * sqrt(lin_reg_all.get_var_b());

	//  evaluate a frequency slope if there was more than one burst
	if (bursts.size() > 1) {
		double max_freq_sigma = 0;
		for (auto &burst: bursts) {
			if (burst.empty()) {
				continue;
			}
			FrequencyMeasurement burst_measurement(burst);
			double burst_freq   = burst_measurement.freq_Hz;
			double burst_time   = burst[0];// burst_lin_reg_all.get_mean_x();
			lin_reg_freq_slope.add_point(burst_time, burst_freq);
			max_freq_sigma = std::max(max_freq_sigma, burst_measurement.freq_sigma_Hz);
		}
		freq_slope_kHz_s       = 1e6*lin_reg_freq_slope.get_b();
		freq_slope_sigma_kHz_s = 1e6*sqrt(lin_reg_freq_slope.get_var_b())*max_freq_sigma;
	}

	valid = true;
}


double test(double freq) {
	double period_ns = 1.0e9/freq;
	double offset_ns = 1.0*rand()/RAND_MAX * 1000;
	std::vector<int64_t> edge_times;
	int I0 = 0;
	int I1 = 200;
	int I2 = 5000;
	for (int i = I0; i < I0+11; ++i) {
		edge_times.push_back(i*period_ns+offset_ns);
	}
	edge_times.back()-=(1.0*rand()/RAND_MAX)*period_ns*0.5;
	for (int i = I1; i < I1+11; ++i) {
		edge_times.push_back(i*period_ns+offset_ns);
	}
	edge_times.back()-=(1.0*rand()/RAND_MAX)*period_ns*0.5;
	for (int i = I2; i < I2+11; ++i) {
		edge_times.push_back(i*period_ns+offset_ns);
	}
	edge_times.back()-=(1.0*rand()/RAND_MAX)*period_ns*0.5;

	FrequencyMeasurement result(edge_times);

	std::cout << "f=" << error_format(result.freq_Hz, result.freq_sigma_Hz) << " Hz \t";
	std::cout << "Rchi2=" << std::setprecision(2) << result.red_chi2 << " \t";
	std::cout << "slope=" << error_format(result.freq_slope_kHz_s, result.freq_slope_sigma_kHz_s) << " kHz/s \t";
	std::cout << "bursts=" << result.num_bursts << " \t";
	std::cout << "points=" << result.num_points << " \t";
	std::cout << "outliers=" << result.num_outliers << " \t";

	std::cout << std::endl;

	return result.freq_Hz;
}


struct DataAcquisition {
	std::vector<int64_t> measurements;
	uint64_t measurement_start_time;
	int SID;
	double expected_period_ns;
	double expected_frequency_Hz;
	std::shared_ptr<saftlib::TimingReceiver_Proxy> timingreceiver;
	int GID;
	bool verbose;
	bool measuring;
	const uint64_t EVALUATE_DATA_EVENT = 0x0000000011011011;

	struct Results {
		double nuSet;
		double nuMean;
		double nuDiff;
		double nuErr;
		double nuRedChi2;
		double nuSlope;
		double nuSlopeErr;
		int    nBurst;
		int    nEedge;
		int    nOutlier;
		Results() {
			nuSet      = 0;
			nuMean     = 0;
			nuDiff     = 0;
			nuErr      = 0;
			nuRedChi2  = 0;
			nuSlope    = 0;
			nuSlopeErr = 0;
			nBurst     = 0;
			nEedge     = 0;
			nOutlier   = 0;
		}
	};
	std::vector<Results>            result_for_sid;
	std::vector<int>                service_ids;     // as returned from dis_add_service()

	std::shared_ptr<saftlib::SoftwareActionSink_Proxy> action_sink;
	DataAcquisition(std::shared_ptr<saftlib::TimingReceiver_Proxy> tr, std::string instance, std::string ring, bool verb) {
		timingreceiver = tr;
		verbose = verb;
		measuring = false;
		result_for_sid.resize(16);
		service_ids.resize(16);

		action_sink = saftlib::SoftwareActionSink_Proxy::create(tr->NewSoftwareActionSink(""));
		uint64_t event, mask;
		if (ring == "sis18") {
			GID = SIS18_B2B_EXTRACT;
		} else if (ring == "esr") {
			GID = ESR_B2B_EXTRACT;
		} else if (ring == "yr") {
			GID = CRYRING_B2B_EXTRACT;
		} else {
			throw std::runtime_error(std::string("DataAcquisition error: unknown ring ") + ring);
		}
                for (int j = 0; j < 2; j++) { // hackish solution for creating the rules for injection into the next ring as well
                  event = make_event(GID+j,B2B_ECADO_B2B_PMEXT);
                  mask  = make_mask (GID+j,B2B_ECADO_B2B_PMEXT);
                  add_condition(event, mask, 0)->SigAction.connect(sigc::mem_fun(this,&DataAcquisition::start_data_taking));
                  add_condition(event, mask, 50000000)->SigAction.connect(sigc::mem_fun(this,&DataAcquisition::finish_measurement));
                } // for j
		event = 0xffffa03000000000;
		mask  = 0xffffffffffffffff;
		add_condition(event, mask, 0)->SigAction.connect(sigc::mem_fun(this,&DataAcquisition::measure_edge));
		/*event = EVALUATE_DATA_EVENT;
		mask  = 0xffffffffffffffff;
		add_condition(event, mask, 0)->SigAction.connect(sigc::mem_fun(this,&DataAcquisition::finish_measurement));*/

		for (int i = 0; i < result_for_sid.size(); ++i) {
			std::ostringstream service_name;
			service_name << "b2b_" << instance << "_" << ring << "-other-rf_sid" << std::setw(2) << std::setfill('0') << std::dec << i << "_ext";
			service_ids[i] = dis_add_service(service_name.str().c_str(), "D:7;I:3", &result_for_sid[i], sizeof(result_for_sid[i]), 0, 0);
			if (verbose) {
				std::cout << "DIM add service: " << service_name.str() << std::endl;
			}
		}
	}

	bool match_event(uint64_t event, int GID, int EVT) {
		return ((event & 0x0fff000000000000) >> (12*4) == GID) && ((event & 0x0000fff000000000) >> (9*4) == EVT);
	}
	int get_sid(uint64_t event) {
		return (event>>20) & 0xfff;
	}
	uint64_t make_event(uint64_t GID) {
		return 0x1000000000000000 | (GID<<(12*4));
	}
	uint64_t make_mask(uint64_t GID) {
		return 0xffff000000000000;
	}
	uint64_t make_event(uint64_t GID, uint64_t EVT) {
		return 0x1000000000000000 | (GID<<(12*4)) | (EVT<<(9*4));
	}
	uint64_t make_mask(uint64_t GID, uint64_t EVT) {
		return 0xfffffff000000000;
	}

	void start_data_taking(uint64_t event, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint8_t flags) {
                if ((match_event(event, GID, B2B_ECADO_B2B_PMEXT) || match_event(event, GID+1, B2B_ECADO_B2B_PMEXT))) { // hackish solution for handling the rules for injection into the next ring as well
			SID = get_sid(event);
			measurements.clear();
			measurement_start_time = deadline.getTAI();
			expected_period_ns = (param & 0x00ffffffffffffff)*1e-9; // extract the set frequency in attoseconds
			expected_frequency_Hz = 1e9/expected_period_ns;
			/* timingreceiver->InjectEvent(EVALUATE_DATA_EVENT, 0x0, deadline+50000000); // evaluate the data 50 ms after the start event */
			measuring = true;
		}
	}

	void measure_edge(uint64_t event, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint8_t flags) {
		if (measuring) {
			measurements.push_back(deadline.getTAI() - measurement_start_time);
		}
	}

	void finish_measurement(uint64_t event, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint8_t flags) {
		if (!measuring) return;
		measuring = false;
		std::sort(measurements.begin(), measurements.end()); // FrequencyMeasurement expects sorted data
		FrequencyMeasurement result(measurements);
		if (result.valid) {
			if (verbose) {
				std::cout << "f=" << error_format(result.freq_Hz, result.freq_sigma_Hz) << " Hz \t";
				std::cout << "Rchi2=" << std::setprecision(2) << result.red_chi2 << " \t";
				std::cout << "slope=" << error_format(result.freq_slope_kHz_s, result.freq_slope_sigma_kHz_s) << " kHz/s \t";
				std::cout << "bursts=" << result.num_bursts << " \t";
				std::cout << "points=" << result.num_points << " \t";
				std::cout << "outliers=" << result.num_outliers << " \t";
				std::cout << "sid=" << SID << " \t";
				std::cout << std::endl;
			}

			result_for_sid[SID].nuSet      = expected_frequency_Hz;
			result_for_sid[SID].nuMean     = result.freq_Hz;
			result_for_sid[SID].nuDiff     = expected_frequency_Hz-result.freq_Hz;
			result_for_sid[SID].nuErr      = result.freq_sigma_Hz;
			result_for_sid[SID].nuRedChi2  = result.red_chi2;
			result_for_sid[SID].nuSlope    = result.freq_slope_kHz_s;
			result_for_sid[SID].nuSlopeErr = result.freq_slope_sigma_kHz_s;
			result_for_sid[SID].nBurst     = result.num_bursts;
			result_for_sid[SID].nEedge     = result.num_points;
			result_for_sid[SID].nOutlier   = result.num_outliers;

		} else { // if measurement was invalid, set all to zero except for the set frequency.
			result_for_sid[SID].nuSet      = expected_frequency_Hz;
			result_for_sid[SID].nuMean     = 0;
			result_for_sid[SID].nuDiff     = 0;
			result_for_sid[SID].nuErr      = 0;
			result_for_sid[SID].nuRedChi2  = 0;
			result_for_sid[SID].nuSlope    = 0;
			result_for_sid[SID].nuSlopeErr = 0;
			result_for_sid[SID].nBurst     = 0;
			result_for_sid[SID].nEedge     = 0;
			result_for_sid[SID].nOutlier   = 0;
		}

		uint64_t measurement_start_time_UTC = saftlib::makeTimeTAI(measurement_start_time).getUTC();
		uint64_t secs  = (measurement_start_time_UTC/1000000)/1000;
		uint64_t msecs = (measurement_start_time_UTC/1000000)%1000;
		dis_set_timestamp(service_ids[SID], secs, msecs);
		dis_update_service(service_ids[SID]);
	}

	std::vector<std::shared_ptr<saftlib::SoftwareCondition_Proxy> > conditions;
  std::shared_ptr<saftlib::SoftwareCondition_Proxy>  add_condition(uint64_t id, uint64_t mask, int64_t myOffset) {
		bool active = true;
		int64_t offset = 0;
		conditions.push_back(saftlib::SoftwareCondition_Proxy::create(action_sink->NewCondition(active, id, mask, offset = myOffset)));
		conditions.back()->setAcceptLate(true);
		conditions.back()->setAcceptEarly(true);
		conditions.back()->setAcceptConflict(true);
		conditions.back()->setAcceptDelayed(true);
		return conditions.back();
	}


};

std::string help(std::string argv0) {
	std::ostringstream out;
	out << "usage: " << argv0 << " <ring> <environment> [options]" << std::endl;
	out << "  <ring>        : either sis18, esr, or yr. Host system is the PM for <ring> extraction."      << std::endl;
	out << "  <environment> : either pro, or int. Host system runs production or integration environment." << std::endl;
	out << std::endl;
	out << " options:"                                                                                     << std::endl;
	out << "  -d <device>  : use a different saftlib device, default is tr0."                              << std::endl;
	out << "  -v --verbose : write measurement results to stdout"                                          << std::endl;
	out << "     --test    : run a self-test and quit"                                                     << std::endl;
	out << "  -h --help    : print this help and exit"                                                     << std::endl;
        out << std::endl;
        out <<  "Version " << FREQ_MEASURE_VERSION << std::endl;
	return out.str();
}

int main(int argc, char **argv){
	try {

		// handle command line arguments
		if (argc <= 1) {
			std::cerr << help(argv[0]) << std::endl;
			return 1;
		}

		std::string ring; // sis18, esr, yr
		std::string instance;
		bool verbose = false;
		std::string device_name = "tr0";


		for (int i = 1; i < argc; ++i) {

			std::string argvi = argv[i];

			if (argvi == "sis18") ring     = argvi;
			if (argvi == "esr")   ring     = argvi;
			if (argvi == "yr")    ring     = argvi;
			if (argvi == "pro")   instance = argvi;
			if (argvi == "int")   instance = argvi;

			if (argvi == "-d") {
				if (++i < argc) {
					device_name = argv[i];
					continue;
				} else {
					std::cerr << "expect device name after -d" << std::endl;
					return 1;
				}
			}
			if (argvi == "-v" || argvi == "--verbose") {
				verbose = true;
				continue;
			}
			if (argvi == "--test" )  { // run a test
				std::ofstream stat_out("stat.dat");
				for (int i = 0 ; i < 10000; ++i) {
					double f = 2.0e6*rand()/RAND_MAX + 0.5e6;
					std::cout << "f=" << std::fixed << std::setprecision(2) << f << std::endl;
					double f_measured = test(f);
					// double f_measured = test_ramp(f,f+5000,f+50000);
					stat_out << f-f_measured << std::endl;
				}
				return 0;
			}
			if (argvi == "--help" || argvi == "-h") {
				std::cout << help(argv[0]) << std::endl;
				return 0;
			}
		}
		if (!ring.size()) {
			std::cerr << "no ring name given. use sis18 or esr or yr" << std::endl;
			return 1;
		}
		if (!instance.size()) {
			std::cerr << "no instance given. use pro or int" << std::endl;
			return 1;
		}


		// saftlib setup
		auto saftd = saftlib::SAFTd_Proxy::create();
		auto devices = saftd->getDevices();
		if (devices.find(device_name) == devices.end()) {
			std::cerr << "cannot find device " << device_name << std::endl;
			return 1;
		}
		auto tr0 = saftlib::TimingReceiver_Proxy::create(devices[device_name]);
		DataAcquisition daq(tr0, instance, ring, verbose);

		// DIM setup
		std::string dim_server_name = "b2b_";
		dim_server_name.append(instance);
		dim_server_name.append("_");
		dim_server_name.append(ring);
		dim_server_name.append("-rf-freq");

		if (verbose) {
			std::cout << "DIM server name: " << dim_server_name << std::endl;
		}

		if (!dis_start_serving(dim_server_name.c_str())) {
			throw std::runtime_error("cannot start DIM server");
		}


		// main loop
		for (;;) {
			saftlib::wait_for_signal(1000);
		}



	} catch(std::runtime_error &e) {
		std::cerr << "error: " << e.what() << std::endl;
		return 2;
	}
	return 0;
}
