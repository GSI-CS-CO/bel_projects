/** Copyright (C) 2018,2023 GSI Helmholtz Centre for Heavy Ion Research GmbH 
 *
 *  @author Michael Reese <m.reese@gsi.de>
 *
 *******************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */

#include "WrMilGateway.hpp"
#include "WrMilGateway_Service.hpp"

#include <SAFTd_Service.hpp>
#include <TimingReceiver_Service.hpp>

#include <saftbus/service.hpp>

#include <memory>
#include <vector>
#include <map>
#include <functional>

extern "C" 
void create_services(saftbus::Container *container, const std::vector<std::string> &args) {
	std::cerr << "create_services args : ";
	for(auto &arg: args) {
		std::cerr << arg << " ";
	}
	std::cerr << std::endl;
	std::string object_path = "/de/gsi/saftlib";
	saftlib::SAFTd_Service *saftd_service = dynamic_cast<saftlib::SAFTd_Service*>(container->get_object(object_path));
	saftlib::SAFTd *saftd = saftd_service->d;


	for(auto &device: args) {
		std::cerr << "install WhiteRabbit-MIL-Gateway firmware for " << device << std::endl;
		std::string device_object_path = object_path;
		device_object_path.append("/");
		device_object_path.append(device);
		saftlib::TimingReceiver_Service *tr_service = dynamic_cast<saftlib::TimingReceiver_Service*>(container->get_object(device_object_path));
		saftlib::TimingReceiver *tr = tr_service->d;

		std::unique_ptr<saftlib::WrMilGateway> wr_mil_gw(new saftlib::WrMilGateway(saftd, tr, container));
		saftlib::WrMilGateway *wr_mil_gw_ptr = wr_mil_gw.get();

		tr->installAddon("WrMilGateway", std::move(wr_mil_gw));
		container->create_object(wr_mil_gw_ptr->getObjectPath(), std::move(std::unique_ptr<saftlib::WrMilGateway_Service>(new saftlib::WrMilGateway_Service(wr_mil_gw_ptr, std::bind(&saftlib::TimingReceiver::removeAddon, tr, "WrMilGateway") ))));
	}

}


