
/* $Id: eodata.cpp 168 2009-10-23 14:58:32Z sausage $
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "eodata.hpp"

#include "EO_Map.hpp"

static const char *safe_fail_filename;

static void safe_fail(int line)
{
#ifdef WIN32
	MessageBox(0, "Invalid file / failed read/seek", "Error", MB_ICONERROR|MB_OK);
#else // WIN32
	fprintf(stderr, "Invalid file / failed read/seek: %s -- %i", safe_fail_filename, line);
#endif // WIN32
	std::exit(1);
}

#define SAFE_SEEK(fh, offset, from) if (std::fseek(fh, offset, from) != 0) { std::fclose(fh); safe_fail(__LINE__); }
#define SAFE_READ(buf, size, count, fh) if (std::fread(buf, size, count, fh) != static_cast<int>(count)) { std::fclose(fh); safe_fail(__LINE__); }

EIF::EIF(std::string filename)
{
	std::FILE *fh = std::fopen(filename.c_str(), "rb");
	safe_fail_filename = filename.c_str();

	if (!fh)
	{
#ifdef WIN32
		MessageBox(0, "Could not load EIF file", "Error", MB_ICONERROR|MB_OK);
#else // WIN32
		fprintf(stderr, "Could not load file: %s", filename.c_str());
#endif // WIN32
		std::exit(1);
	}

	SAFE_SEEK(fh, 3, SEEK_SET);
	SAFE_READ(this->rid, sizeof(char), 4, fh);
	SAFE_READ(this->len, sizeof(char), 2, fh);
	int numobj = EON(this->len[0], this->len[1]);
	SAFE_SEEK(fh, 1, SEEK_CUR);

	unsigned char namesize;
	char *namebuf;
	std::string name;
	char buf[EIF::DATA_SIZE] = {0};
	EIF_Data newdata;

	this->data.resize(numobj+1, newdata);

	this->data[0] = newdata;

	SAFE_READ(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	for (int i = 1; i <= numobj; ++i)
	{
		namesize = EON(namesize);
		namebuf = new char[namesize];
		SAFE_READ(namebuf, sizeof(char), namesize, fh);
		name.assign(namebuf,namesize);
		delete[] namebuf;
		SAFE_READ(buf, sizeof(char), EIF::DATA_SIZE, fh);

		newdata.id = i;
		newdata.name = name;

		newdata.graphic = EON(buf[0], buf[1]);
		newdata.type = static_cast<EIF::Type>(EON(buf[2]));
		newdata.subtype = static_cast<EIF::SubType>(EON(buf[3]));
		// Ranged gun hack
		if (newdata.id == 365 && newdata.name == "Gun")
		{
			newdata.subtype = EIF::Ranged;
		}
		// / Ranged gun hack
		newdata.special = static_cast<EIF::Special>(EON(buf[4]));
		newdata.hp = EON(buf[5], buf[6]);
		newdata.tp = EON(buf[7], buf[8]);
		newdata.mindam = EON(buf[9], buf[10]);
		newdata.maxdam = EON(buf[11], buf[12]);
		newdata.accuracy = EON(buf[13], buf[14]);
		newdata.evade = EON(buf[15], buf[16]);
		newdata.armor = EON(buf[17], buf[18]);
		newdata.str = EON(buf[20]);
		newdata.intl = EON(buf[21]);
		newdata.wis = EON(buf[22]);
		newdata.agi = EON(buf[23]);
		newdata.con = EON(buf[24]);
		newdata.cha = EON(buf[25]);
		newdata.scrollmap = EON(buf[32]);
		newdata.scrollx = EON(buf[35]);
		newdata.scrolly = EON(buf[36]);

		newdata.classreq = EON(buf[39]);

		newdata.weight = EON(buf[55]);

		this->data[i] = newdata;

		if (std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh) != 1)
		{
			break;
		}
	}

	if (newdata.name.compare("eof") == 0)
	{
		this->data.pop_back();
	}

	//Console::Out("%i items loaded.", this->data.size()-1);

	std::fclose(fh);
}

EIF_Data *EIF::Get(unsigned int id)
{
	if (id > 0 && id < this->data.size())
	{
		return &this->data[id];
	}
	else
	{
		return &this->data[0];
	}
}

ENF::ENF(std::string filename)
{
	std::FILE *fh = std::fopen(filename.c_str(), "rb");
	safe_fail_filename = filename.c_str();

	if (!fh)
	{
#ifdef WIN32
		MessageBox(0, "Could not load ENF file", "Error", MB_ICONERROR|MB_OK);
#else // WIN32
		fprintf(stderr, "Could not load file: %s", filename.c_str());
#endif // WIN32
		std::exit(1);
	}

	SAFE_SEEK(fh, 3, SEEK_SET);
	SAFE_READ(this->rid, sizeof(char), 4, fh);
	SAFE_READ(this->len, sizeof(char), 2, fh);
	int numobj = EON(this->len[0], this->len[1]);
	SAFE_SEEK(fh, 1, SEEK_CUR);

	unsigned char namesize;
	char *namebuf;
	std::string name;
	char buf[ENF::DATA_SIZE] = {0};
	ENF_Data newdata;

	this->data.resize(numobj+1, newdata);

	this->data[0] = newdata;

	SAFE_READ(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	for (int i = 1; i <= numobj; ++i)
	{
		namesize = EON(namesize);
		namebuf = new char[namesize];
		SAFE_READ(namebuf, sizeof(char), namesize, fh);
		name.assign(namebuf,namesize);
		delete[] namebuf;
		SAFE_READ(buf, sizeof(char), ENF::DATA_SIZE, fh);

		newdata.id = i;
		newdata.name = name;

		newdata.graphic = EON(buf[0], buf[1]);

		newdata.boss = EON(buf[3], buf[4]);
		newdata.child = EON(buf[5], buf[6]);
		newdata.type = static_cast<ENF::Type>(EON(buf[7], buf[8]));
		newdata.hp = EON(buf[11], buf[12], buf[13]);

		newdata.mindam = EON(buf[16], buf[17]);
		newdata.maxdam = EON(buf[18], buf[19]);

		newdata.accuracy = EON(buf[20], buf[21]);
		newdata.evade = EON(buf[22], buf[23]);
		newdata.armor = EON(buf[24], buf[25]);

		newdata.exp = EON(buf[36], buf[37]);

		this->data[i] = newdata;

		if (std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh) != 1)
		{
			break;
		}
	}

	if (newdata.name.compare("eof") == 0)
	{
		this->data.pop_back();
	}

	//Console::Out("%i npc types loaded.", this->data.size()-1);

	std::fclose(fh);
}

ENF_Data *ENF::Get(unsigned int id)
{
	if (id > 0 && id < this->data.size())
	{
		return &this->data[id];
	}
	else
	{
		return &this->data[0];
	}
}

ESF::ESF(std::string filename)
{
	std::FILE *fh = std::fopen(filename.c_str(), "rb");
	safe_fail_filename = filename.c_str();

	if (!fh)
	{
#ifdef WIN32
		MessageBox(0, "Could not load ESF file", "Error", MB_ICONERROR|MB_OK);
#else // WIN32
		fprintf(stderr, "Could not load file: %s", filename.c_str());
#endif // WIN32
		std::exit(1);
	}

	SAFE_SEEK(fh, 3, SEEK_SET);
	SAFE_READ(this->rid, sizeof(char), 4, fh);
	SAFE_READ(this->len, sizeof(char), 2, fh);
	int numobj = EON(this->len[0], this->len[1]);
	SAFE_SEEK(fh, 1, SEEK_CUR);

	unsigned char namesize, shoutsize;
	char *namebuf, *shoutbuf;
	std::string name, shout;
	char buf[ESF::DATA_SIZE] = {0};
	ESF_Data newdata;

	this->data.resize(numobj+1, newdata);

	this->data[0] = newdata;

	SAFE_READ(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	SAFE_READ(static_cast<void *>(&shoutsize), sizeof(char), 1, fh);
	for (int i = 1; i <= numobj; ++i)
	{
		namesize = EON(namesize);
		namebuf = new char[namesize];
		SAFE_READ(namebuf, sizeof(char), namesize, fh);
		name.assign(namebuf, namesize);
		delete[] namebuf;

		shoutsize = EON(shoutsize);
		shoutbuf = new char[shoutsize];
		SAFE_READ(shoutbuf, sizeof(char), shoutsize, fh);
		shout.assign(shoutbuf, shoutsize);
		delete[] shoutbuf;

		SAFE_READ(buf, sizeof(char), ESF::DATA_SIZE, fh);

		newdata.id = i;
		newdata.name = name;
		newdata.shout = shout;

		this->data[i] = newdata;

		if (std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh) != 1)
		{
			break;
		}

		if (std::fread(static_cast<void *>(&shoutsize), sizeof(char), 1, fh) != 1)
		{
			break;
		}
	}

	if (this->data[numobj-1].name.compare("eof") == 0)
	{
		this->data.pop_back();
	}

	//Console::Out("%i spells loaded.", this->data.size()-1);

	std::fclose(fh);
}

ECF::ECF(std::string filename)
{
	std::FILE *fh = std::fopen(filename.c_str(), "rb");
	safe_fail_filename = filename.c_str();

	if (!fh)
	{
#ifdef WIN32
		MessageBox(0, "Could not load ECF file", "Error", MB_ICONERROR|MB_OK);
#else // WIN32
		fprintf(stderr, "Could not load file: %s", filename.c_str());
#endif // WIN32
		std::exit(1);
	}

	SAFE_SEEK(fh, 3, SEEK_SET);
	SAFE_READ(this->rid, sizeof(char), 4, fh);
	SAFE_READ(this->len, sizeof(char), 2, fh);
	int numobj = EON(this->len[0], this->len[1]);
	SAFE_SEEK(fh, 1, SEEK_CUR);

	unsigned char namesize;
	char *namebuf;
	std::string name;
	char buf[ECF::DATA_SIZE] = {0};
	ECF_Data newdata;

	this->data.resize(numobj+1, newdata);

	this->data[0] = newdata;

	SAFE_READ(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	for (int i = 1; i <= numobj; ++i)
	{
		namesize = EON(namesize);
		namebuf = new char[namesize];
		SAFE_READ(namebuf, sizeof(char), namesize, fh);
		name.assign(namebuf,namesize);
		delete[] namebuf;
		SAFE_READ(buf, sizeof(char), ECF::DATA_SIZE, fh);

		newdata.id = i;
		newdata.name = name;

		this->data[i] = newdata;

		if (std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh) != 1)
		{
			break;
		}
	}

	if (newdata.name.compare("eof") == 0)
	{
		this->data.pop_back();
	}

	//Console::Out("%i classes loaded.", this->data.size()-1);

	std::fclose(fh);
}

