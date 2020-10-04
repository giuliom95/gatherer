#include "gathereddata.hpp"

void GatheredData::loadall(const boost::filesystem::path& folder)
{
	
	datafolder = folder;

	const boost::filesystem::path lengths_fp = 
		folder / "paths" / "lengths.bin";
	const boost::filesystem::path positions_fp = 
		folder / "bounces" / "positions.bin";
	const boost::filesystem::path luminance_fp = 
		folder / "paths" / "luminance.bin";
	const boost::filesystem::path camerasamples_fp = 
		folder / "paths" / "camerasamples.bin";

	boost::filesystem::ifstream lengths_ifs(lengths_fp);
	boost::filesystem::ifstream positions_ifs(positions_fp);
	boost::filesystem::ifstream luminance_ifs(luminance_fp);
	boost::filesystem::ifstream camerasamples_ifs(camerasamples_fp);
	
	const unsigned lenghts_bytesize   = boost::filesystem::file_size(lengths_fp);
	const unsigned positions_bytesize = boost::filesystem::file_size(positions_fp);

	npaths = lenghts_bytesize / sizeof(uint8_t);
	nbounces = positions_bytesize / sizeof(Vec3h);

	pathslength.resize(npaths);
	bouncesposition.resize(nbounces);
	pathsluminance.resize(npaths);
	pathscamerasamples.resize(npaths);

	lengths_ifs.read((char*)pathslength.data(), lenghts_bytesize);
	positions_ifs.read((char*)bouncesposition.data(), positions_bytesize);
	luminance_ifs.read((char*)pathsluminance.data(), npaths * sizeof(Vec3h));
	camerasamples_ifs.read((char*)pathscamerasamples.data(), npaths * sizeof(CameraSample));

	lengths_ifs.close();
	positions_ifs.close();
	luminance_ifs.close();
	camerasamples_ifs.close();

	firstbounceindexes.reserve(npaths);
	unsigned off = 0;
	for(uint8_t l : pathslength)
	{
		firstbounceindexes.push_back(off);
		off += l;
	}

	BOOST_LOG_TRIVIAL(info) << "Loaded all paths";

	boost::filesystem::path json_path = folder / "scene.json";

	nlohmann::json json_data;
	boost::filesystem::ifstream json_file{json_path};
	if(!json_file) 
	{
		LOG(fatal) <<
			"Could not open \"" << json_path.string() << "\"";
		throw std::runtime_error("Could not open scene file");
	}
	json_file >> json_data;
	json_file.close();

	rendersamples = json_data["render"]["spp"];

	rendersize = Vec2i{
		json_data["render"]["width"],
		json_data["render"]["height"]
	};

	selectedpaths.reserve(npaths);
	selectedpaths.reserve(npaths);
}