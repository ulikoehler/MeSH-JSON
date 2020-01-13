#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <pugixml.hpp>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/ostreamwrapper.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
using namespace boost::algorithm;
using namespace std;
using namespace pugi;
using namespace rapidjson;

MemoryPoolAllocator<> jsonAlloc;

Document parseQualifierList(const xml_node& qualifierList) {
    // Return an array of qualifier objects
    Document array(kArrayType);
    for(const auto& qualifier : qualifierList.children("AllowableQualifier")) {
        xml_node ref = qualifier.child("QualifierReferredTo");
        auto id = ref.child("QualifierUI").child_value();
        auto name = ref.child("QualifierName").child("String").child_value();
        // Build JSON
        Document doc;
        doc.SetObject(); // Make doc an object !
        // Add JSON entries
        doc.AddMember("id", Value().SetString(id, jsonAlloc), jsonAlloc);
        doc.AddMember("name", Value().SetString(name, jsonAlloc), jsonAlloc);
        // Add doc to array
        array.PushBack(doc, jsonAlloc);
    }
    return array;
}

Document parseConceptRelationList(const xml_node& conceptRelationList, const string& conceptID) {
    Document array(kArrayType);
    for(const auto& relation : conceptRelationList.children("ConceptRelation")) {
        string relName = relation.attribute("RelationName").value();
        // These three are mutually exclusive
        // See https://www.nlm.nih.gov/mesh/xml_data_elements.html
        bool broader = relName == "BRD";
        bool narrower = relName == "NRW";
        bool related = relName == "REL";
        // Assumption: Concept2UI is always the other concept
        // (this assumption is checked)
        string c1ui = relation.child("Concept1UI").child_value();
        string c2ui = relation.child("Concept2UI").child_value();
        string other;
        if(conceptID == c1ui) {
            other = c2ui;
        } else if(conceptID == c2ui) {
            other = c1ui;
        } else {
            assert(false); // current concept is not in relation
        }
        // Build JSON
        Document doc;
        doc.SetObject(); // Make doc an object !
        // Add JSON entries
        doc.AddMember("other", Value().SetString(other.c_str(), other.size(), jsonAlloc), jsonAlloc);
        if(broader) {
            doc.AddMember("type", "broader", jsonAlloc);
        } else if(narrower) {
            doc.AddMember("type", "narrower", jsonAlloc);
        } else if(related) {
            doc.AddMember("type", "related", jsonAlloc);
        } else {
            cerr << "Unknown relation type, please add to code: " << relName << endl;
            assert(false); // Unknown type
        }
        // Add doc to array
        array.PushBack(doc, jsonAlloc);
    }
    return array;
}

Document parseTermList(const xml_node& termList) {
    Document array;
    array.SetArray();
    for(const auto& term : termList.children("Term")) {
        auto id = term.child("TermUI").child_value();
        auto name = term.child("String").child_value();
        // Build JSON
        Document doc;
        doc.SetObject(); // Make doc an object !
        // Add JSON entries
        doc.AddMember("id", Value().SetString(id, jsonAlloc), jsonAlloc);
        doc.AddMember("name", Value().SetString(name, jsonAlloc), jsonAlloc);
        // Add doc to array
        array.PushBack(doc, jsonAlloc);
    }
    return array;
}

Document parseConceptList(const xml_node& conceptList) {
    Document array;
    array.SetArray();
    for(const auto& concept : conceptList.children("Concept")) {
        Document doc;
        doc.SetObject(); // Make doc an object !
        bool isPreferred = concept.attribute("PreferredConceptYN").value() == string("Y");
        auto id = concept.child("ConceptUI").child_value();
        auto casn1Name = concept.child("CASN1Name").child_value();
        auto name = concept.child("ConceptName").child("String").child_value();
        auto note = concept.child("ScopeNote").child_value();
        string trimmedNote = trim_right_copy(string(note));
        // Terms
        doc.AddMember("terms", parseTermList(concept.child("TermList")), jsonAlloc);
        // Relations
        doc.AddMember("relations",
            parseConceptRelationList(concept.child("ConceptRelationList"), id), jsonAlloc);
        // Add JSON entries
        doc.AddMember("id", Value().SetString(id, jsonAlloc), jsonAlloc);
        doc.AddMember("name", Value().SetString(name, jsonAlloc), jsonAlloc);
        doc.AddMember("isPreferred", Value().SetBool(isPreferred), jsonAlloc);
        doc.AddMember("note", Value().SetString(
            trimmedNote.c_str(), trimmedNote.size(), jsonAlloc), jsonAlloc);
        if(!string(casn1Name).empty()) {
            doc.AddMember("CASN1Name", Value().SetString(casn1Name, jsonAlloc), jsonAlloc);
        }
        // Add doc to array
        array.PushBack(doc, jsonAlloc);
    }
    return array;
}

Document parseDescriptorRecord(const xml_node& node) {
    Document doc;
    doc.SetObject();
    auto id = node.child("DescriptorUI").child_value();
    auto name = node.child("DescriptorName").child("String").child_value();
    auto cls = node.attribute("DescriptorClass").value();
    // Add JSON entries
    doc.AddMember("id", Value().SetString(id, jsonAlloc), jsonAlloc);
    doc.AddMember("name", Value().SetString(name, jsonAlloc), jsonAlloc);
    doc.AddMember("class", Value().SetInt(
        boost::lexical_cast<int>(cls)), jsonAlloc);
    // Parse qualifiers & concepts
    doc.AddMember("qualifiers", parseQualifierList(node.child("AllowableQualifiersList")), jsonAlloc);
    doc.AddMember("concepts", parseConceptList(node.child("ConceptList")), jsonAlloc);
    return doc;
}

void parseDescriptorRecordSet(const xml_node& node, const char* outfile) {
    // Write JSON to file
    FILE* fout = fopen(outfile, "w");
    char writeBuffer[65536];
    FileWriteStream os(fout, writeBuffer, sizeof(writeBuffer));
 
    bool first = true;
    for(const auto& child : node.children()) {
        Writer<FileWriteStream> writer(os); // Writer may only be used once !
        auto doc = parseDescriptorRecord(child);
        doc.Accept(writer);
        // Separate JSONs with newline character
        if(!first) {
            os.Put('\n');
        }
        first = false;
    }
    // Cleanup
    fclose(fout);
}

int main(int argc, char** argv) {
    if(argc <= 2) { // <= (number of expected CLI arguments)
        fprintf(stderr, "Usage: %s <input .xml.gz file> <output JSON file>\n", argv[0]);
        return -1;
    }
    
    // Open "raw" gzipped data stream
    ifstream file(argv[1], ios_base::in | ios_base::binary);
    // Configure decompressor filter
    boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
    inbuf.push(boost::iostreams::gzip_decompressor());
    inbuf.push(file);
    //Convert streambuf to istream
    istream instream(&inbuf);
    // Parse from stream
    xml_document doc;
    xml_parse_result result = doc.load(instream);

    parseDescriptorRecordSet(doc.child("DescriptorRecordSet"), argv[2]);
}
