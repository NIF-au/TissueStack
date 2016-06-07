/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "utils.h"
#include "logging.h"

tissuestack::utils::Misc::Misc() {}

const std::string tissuestack::utils::Misc::convertCharPointerToString(const char * some_characters)
{
	if (some_characters == nullptr) return std::string("");

	std::ostringstream in;
	in << some_characters;

	return in.str();
}

const std::string tissuestack::utils::Misc::demangleTypeIdName(const char * mangeledIdName)
{
	if (mangeledIdName == nullptr) return std::string("");

	int status = 0;
	char * buffer = NULL;
	unsigned long int length = strlen(mangeledIdName);

	char * demangled_name = abi::__cxa_demangle(mangeledIdName, buffer, &length, &status);
	if (status != 0) return std::string("");

	// copy over result
	std::string ret(tissuestack::utils::Misc::convertCharPointerToString(demangled_name));
	free(demangled_name);

	return ret;
}

const std::string tissuestack::utils::Misc::maskQuotesInJson(const std::string & json)
{
	if (json.empty()) return json;

	std::ostringstream in;
	for (char c : json)
	{
		if (c == '"')
			in << "\\\"";
		else if (c == '\\')
			in << "\\\\";
		else if (c == '\r')
			in << "\\r";
		else if (c == '\b')
			in << "\\b";
		else if (c == '\f')
			in << "\\f";
		else if (c == '\t')
			in << "\\t";
		else if (c == '\n')
			in << "\\n";
		else
			in << c;
	}
	return in.str();
}

const std::string tissuestack::utils::Misc::sanitizeSqlQuote(const std::string & quoted_value)
{
	if (quoted_value.empty()) return quoted_value;

	std::ostringstream in;
	for (char c : quoted_value)
	{
		if (c == '\'')
			in << "''";
		else
			in << c;
	}
	return in.str();
}


const std::string tissuestack::utils::Misc::eraseCharacterFromString(const std::string & someString, const char unwantedCharacter)
{
	std::ostringstream in;

	for(char c : someString)
		if (c != unwantedCharacter)
			in << c;

	return in.str();
}


const std::string tissuestack::utils::Misc::eliminateWhitespaceAndUnwantedEscapeCharacters(const std::string & someString)
{
	std::ostringstream in;

	for(char c : someString)
		if (c != ' ' &&  c != '\t' && c != '\r' && c != '\n')
			in << c;

	return in.str();
}

const std::vector<std::string> tissuestack::utils::Misc::tokenizeString(const std::string & some_string, const char delimiter)
{
	std::vector<std::string> tokens;

	if (some_string.empty())
		return tokens;

	size_t old_pos = 0;
	size_t pos = 0;

	while (1)
	{
		// peek ahead to avoid repeated appearances of the delimiter
		if (pos != std::string::npos &&
				pos < some_string.length()-1 &&
				some_string.at(pos) == delimiter &&
				some_string.at(pos+1) == delimiter)
		{
			++pos;
			++old_pos;
			continue;
		}

		pos = some_string.find(delimiter, pos);
		if (pos != std::string::npos && pos == old_pos)
		{
			++pos;
			continue;
		}

		tokens.push_back(some_string.substr(old_pos, pos-old_pos));
		if (pos == std::string::npos)
			break;

		++pos;
		old_pos=pos;
	}

	return tokens;
}

const std::string tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(
		const std::unordered_map<std::string, std::string> & map, std::string key)
{
	std::transform(key.begin(), key.end(), key.begin(), toupper);

	try
	{
		return map.at(key);
	} catch (const std::out_of_range & ignored) { }

	return std::string("");
}

const std::string tissuestack::utils::Misc::composeHttpResponse(
		const std::string status, const std::string content_type, const std::string content, const bool gzipped)
{
	const std::string CR_LF = "\r\n";
	std::ostringstream response;

	response << "HTTP/1.1 " << status << CR_LF; // HTTTP/1.1 status
	response << "Date: Thu, 20 May 2004 21:12:11 GMT" << CR_LF; // Date (in the past)
	response << "Connection: close" << CR_LF; // Connection header (close)
	response << "Server: Tissue Stack Image Server" <<  CR_LF; // Server header
	response << "Accept-Ranges: bytes" << CR_LF; // Accept-Ranges header
	response << "Content-Type: " << content_type << CR_LF; // Content-Type header
	if (gzipped) response << "Content-Encoding: gzip" << CR_LF; // if gzipped
	response << "Last-Modified: Thu, 20 May 2004 21:12:11 GMT" << CR_LF; // last modified header in the past
	response << "Access-Control-Allow-Origin: *" << CR_LF; // allow cross origin requests

	if (!content.empty())
	{
		response << "Content-Length: " << content.length() << CR_LF << CR_LF; // Content-Length header
		response << content;
	} else response <<  CR_LF;

	return response.str();
}

const bool tissuestack::utils::Misc::streamGzippedDataToDescriptor(unsigned char * data, const unsigned int length, const int descriptor)
{
	const unsigned int CHUNK = 16384;
	unsigned char out[CHUNK];
	z_stream strm;
	int ret = 0;

	// initialize
	strm.zalloc = Z_NULL;
	strm.zfree  = Z_NULL;
	strm.opaque = Z_NULL;
	ret = deflateInit2(
		&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY);
	if (ret < 0)
		return false;

	strm.next_in = data;
	strm.avail_in = length;
	strm.next_out = out;
	strm.avail_out = CHUNK;

	int flush = Z_NO_FLUSH;
	bool finished = false;
	do
	{
		if (strm.avail_out != 0)
		{
			flush = Z_FINISH;
			finished = true;
		}

		strm.next_out = out;
		strm.avail_out = CHUNK;

		ret = deflate(&strm, flush);
		if (ret < 0)
			return false;

		 const unsigned int bytesToBeWritten =
			CHUNK - strm.avail_out;

		 ssize_t bWritten =
			write(descriptor, out, bytesToBeWritten);

		 if (bWritten != bytesToBeWritten)
			 return false;
	} while (strm.avail_out == 0 || !finished);

	deflateEnd (& strm);

	return true;
}

const std::vector<std::string> tissuestack::utils::Misc::getContentsOfZipArchive(const std::string & archive)
{
	std::vector<std::string> archiveContents;

	if (!tissuestack::utils::System::fileExists(archive))
		return archiveContents;

	 int err = 0;
	 zip * zipped_archive =
		zip_open(archive.c_str(), 0, &err);
	 if (zipped_archive == NULL)
		 return archiveContents;

	 uint64_t numArchFiles =
		zip_get_num_files(zipped_archive);
	 if (numArchFiles == 0)
	 {
		 zip_close(zipped_archive);
		 return archiveContents;
	 }

	 struct zip_stat zippedFile;
	 zip_stat_init(&zippedFile);

	 for (uint64_t i=0;i<numArchFiles;i++)
		 if (zip_stat_index(zipped_archive, i, 0, &zippedFile) == 0)
			 archiveContents.push_back(std::string(zippedFile.name));

	 zip_close(zipped_archive);

	 return archiveContents;
}

const bool tissuestack::utils::Misc::extractZippedFileFromArchive(
	const std::string & archive,
	const std::string & file_to_be_extracted,
	const std::string & new_file_location,
	const bool & overwriteExistingFile)
{
	if (archive.empty() || file_to_be_extracted.empty() || new_file_location.empty())
		return false;

	if (tissuestack::utils::System::fileExists(new_file_location)) // destination exists
	{
		if (overwriteExistingFile && (unlink(new_file_location.c_str()) < 0))
			return false; // delete was tried but failed
		else if (!overwriteExistingFile)
			return false;
	}

	// check if our directory path exists
	size_t position = new_file_location.rfind("/");
	if (position == std::string::npos)
		return false;

	const std::string destDir = new_file_location.substr(0,position);
	if (!tissuestack::utils::System::directoryExists(destDir) && // try to create the directory
		!tissuestack::utils::System::createDirectory(destDir, 0755))
		return false;

	// now open the archive and extract our file
	int err = 0;
	zip * zipped_archive =
			zip_open(archive.c_str(), 0, &err);
	if (zipped_archive == NULL)
		return false;

    struct zip_stat zippedFileInfo;
    zip_stat_init(&zippedFileInfo);
    if (zip_stat(zipped_archive, file_to_be_extracted.c_str(), 0, &zippedFileInfo) < 0)
    {
    	zip_close(zipped_archive);
    	return false;
    }

    int fd = open(new_file_location.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd <= 0)
    {
    	zip_close(zipped_archive);
    	return false;
    }
    zip_file * zippedFile = zip_fopen(zipped_archive, file_to_be_extracted.c_str(), 0);

    // read in chunks
    char buffer[1024];
    uint64_t bytesLeft = zippedFileInfo.size;
	int bytesRead = 0;
    while (bytesLeft > 0)
    {
    	bytesRead = zip_fread(zippedFile, &buffer, 1024);
    	if (bytesRead < 0)
    	{
    		close(fd);
    		unlink(new_file_location.c_str());
    		zip_fclose(zippedFile);
    		zip_close(zipped_archive);
    		return false;
    	}
    	bytesLeft = bytesLeft - bytesRead;
    	write(fd, buffer, bytesRead);
    }
    close(fd);
	zip_fclose(zippedFile);
	zip_close(zipped_archive);

	return true;
}

const bool tissuestack::utils::Misc::isNumber(const std::string some_string) {
	if (some_string.empty()) return false;

	for(auto c : some_string)
		if (isdigit(c) == 0) return false;

	return true;
}
