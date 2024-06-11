// 1. ./myConverter records.csv records.dat 1
// 2. ./myConverter records.dat records.xml 2
// 3. ./myConverter records.xml records.xsd 3



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include <libxml/xmlschemastypes.h>

#define MAX_LINE_LENGTH 100
#define MAX_STUDENT_COUNT 100

// struct for student info
typedef struct {
    char name[20]; // 19 char + null 
    char surname[30]; 
    char stuID[11]; 
    char gender;
    char email[31]; 
    char phone[18]; 
    char letter_grade[3]; 
    int midterm;
    int project;
    int final;
    char regularStudent[5]; 
    int course_surveyRating;
} Student;

// read csv and transfer to struct
int read_csv_file(const char *filename, Student students[], int max_students) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return -1;
    }

    char line[MAX_LINE_LENGTH];
    int student_count = 0;

    // read csv file
    while (fgets(line, sizeof(line), file) && student_count < max_students) {
        // skip first row 
        if (student_count == 0) {
            student_count++;
            continue;
        }
            //transfer to struct
        sscanf(line, "%[^,],%[^,],%[^,],%c,%[^,],%[^,],%[^,],%d,%d,%d,%[^,],%d\n",
               students[student_count - 1].name,
               students[student_count - 1].surname,
               students[student_count - 1].stuID,
               &students[student_count - 1].gender,
               students[student_count - 1].email,
               students[student_count - 1].phone,
               students[student_count - 1].letter_grade,
               &students[student_count - 1].midterm,
               &students[student_count - 1].project,
               &students[student_count - 1].final,
               students[student_count - 1].regularStudent,
               &students[student_count - 1].course_surveyRating);

        student_count++;
    }

    fclose(file);
    return student_count- 1; 
}

// read json
int read_json_file(const char *filename, char *dataFileName, int *keyStart, int *keyEnd, char *order) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *jsonData = (char *)malloc(fileSize + 1);
    fread(jsonData, 1, fileSize, file);
    fclose(file);

    jsonData[fileSize] = '\0';

    // parse json data
    struct json_object *jsonObj = json_tokener_parse(jsonData);

    
    struct json_object *dataFileNameObj, *keyStartObj, *keyEndObj, *orderObj;
    json_object_object_get_ex(jsonObj, "dataFileName", &dataFileNameObj);
    json_object_object_get_ex(jsonObj, "keyStart", &keyStartObj);
    json_object_object_get_ex(jsonObj, "keyEnd", &keyEndObj);
    json_object_object_get_ex(jsonObj, "order", &orderObj);

    strcpy(dataFileName, json_object_get_string(dataFileNameObj));
    *keyStart = json_object_get_int(keyStartObj);
    *keyEnd = json_object_get_int(keyEndObj);
    strcpy(order, json_object_get_string(orderObj));

    free(jsonData);
    return 0;
}

// convert to binary
int csv_to_binary(const char *csv_filename, const char *binary_filename) {
    Student students[MAX_STUDENT_COUNT];
    int student_count = read_csv_file(csv_filename, students, MAX_STUDENT_COUNT);

    if (student_count <= 0) {
        fprintf(stderr, "Error reading CSV file.\n");
        return -1;
    }

    FILE *binary_file = fopen(binary_filename, "wb");
    if (!binary_file) {
        fprintf(stderr, "Error opening binary file for writing.\n");
        return -1;
    }

    fwrite(students, sizeof(Student), student_count, binary_file);
    fclose(binary_file);

    printf("CSV file %s is written to binary file succesfully.\n", binary_filename);
    return 0;
}

// convert to xml
int binary_to_xml(const char *binary_filename, const char *json_filename) {
    // read json settings
    char dataFileName[100];
    int keyStart, keyEnd;
    char order[5];
    if (read_json_file(json_filename, dataFileName, &keyStart, &keyEnd, order) != 0) {
        fprintf(stderr, "Error reading JSON file.\n");
        return -1;
    }

    // read binary file
    FILE *binary_file = fopen(dataFileName, "rb");
    if (!binary_file) {
        fprintf(stderr, "Error opening binary file for reading.\n");
        return -1;
    }

    Student students[MAX_STUDENT_COUNT];
    int student_count = fread(students, sizeof(Student), MAX_STUDENT_COUNT, binary_file);
    fclose(binary_file);

    // write xml file
    FILE *xml_file = fopen("records.xml", "w");
    if (!xml_file) {
        fprintf(stderr, "Error opening XML file for writing.\n");
        return -1;
    }

    fprintf(xml_file, "<%s>\n", "records");

    for (int i = 0; i < student_count; i++) {
        // skip first row
        if (i == 0) {
            continue;
        }

        fprintf(xml_file, "\t<row id=\"%d\">\n", i);
        fprintf(xml_file, "\t<student_info>\n");
        fprintf(xml_file, "\t\t<name>%s</name>\n", students[i].name);
        fprintf(xml_file, "\t\t<surname>%s</surname>\n", students[i].surname);
        fprintf(xml_file, "\t\t<stuID>%s</stuID>\n", students[i].stuID);
        fprintf(xml_file, "\t\t<gender>%c</gender>\n", students[i].gender);
        fprintf(xml_file, "\t\t<email>%s</email>\n", students[i].email);
        fprintf(xml_file, "\t\t<phone>%s</phone>\n", students[i].phone);
        fprintf(xml_file, "\t</student_info>\n");
        fprintf(xml_file, "\t<grade_info letter_grade=\"%s\">\n", students[i].letter_grade);
        fprintf(xml_file, "\t\t<midterm>%d</midterm>\n", students[i].midterm);
        fprintf(xml_file, "\t\t<project>%d</project>\n", students[i].project);
        fprintf(xml_file, "\t\t<final>%d</final>\n", students[i].final);
        fprintf(xml_file, "\t</grade_info>\n");
        fprintf(xml_file, "\t\t<regularStudent>%s</regularStudent>\n", students[i].regularStudent);
        fprintf(xml_file, "\t\t<course_surveyRating>%d</course_surveyRating>\n", students[i].course_surveyRating);
        fprintf(xml_file, "\t</row>\n");
    }

    fprintf(xml_file, "</%s>\n", "records");

    fclose(xml_file);

    printf("Datas where binary file %s are writtten to xml file succesfully.\n", dataFileName);
    return 0;
}
    //validate xml and xsd from lab 6
int validateXml(const char *XMLFileName, const char *XSDFileName) {
    xmlDocPtr doc;
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;
	
    xmlLineNumbersDefault(1); //set line numbers, 0> no substitution, 1>substitution
    ctxt = xmlSchemaNewParserCtxt(XSDFileName); //create an xml schemas parse context
    schema = xmlSchemaParse(ctxt); //parse a schema definition resource and build an internal XML schema
    xmlSchemaFreeParserCtxt(ctxt); //free the resources associated to the schema parser context
    
    doc = xmlReadFile(XMLFileName, NULL, 0); //parse an XML file
    if (doc == NULL)
    {
        fprintf(stderr, "Could not parse %s\n", XMLFileName);
    }
    else
    {
        xmlSchemaValidCtxtPtr ctxt;  //structure xmlSchemaValidCtxt, not public by API
        int ret;
        
        ctxt = xmlSchemaNewValidCtxt(schema); //create an xml schemas validation context 
        ret = xmlSchemaValidateDoc(ctxt, doc); //validate a document tree in memory
        if (ret == 0) //validated
        {
            printf("%s validates\n", XMLFileName);
        }
        else if (ret > 0) //positive error code number
        {
            printf("%s fails to validate\n", XMLFileName);
        }
        else //internal or API error
        {
            printf("%s validation generated an internal error\n", XMLFileName);
        }
        xmlSchemaFreeValidCtxt(ctxt); //free the resources associated to the schema validation context
        xmlFreeDoc(doc);
    }
    // free the resource
    if(schema != NULL)
        xmlSchemaFree(schema); //deallocate a schema structure

    xmlSchemaCleanupTypes(); //cleanup the default xml schemas types library
    xmlCleanupParser(); //cleans memory allocated by the library itself 
    xmlMemoryDump(); //memory dump
    return(0);
}



int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <csv_filename> <binary_filename> <operation>\n", argv[0]);
        return 1;
    }

    const char *csv_filename = argv[1];
    const char *binary_filename = argv[2];
    int operation = atoi(argv[3]);

    //first operation
    if (operation == 1) {
        if (csv_to_binary(csv_filename, binary_filename) != 0) {
            fprintf(stderr, "Error converting CSV to binary.\n");
            return 1;
        }
    } else if (operation == 2) {  //second operation
        const char *json_filename = "setupParams.json";
        if (binary_to_xml(binary_filename, json_filename) != 0) {
            fprintf(stderr, "Error converting binary to XML.\n");
            return 1;
        }
    } else if (operation == 3) { //third operation
        const char *xml_filename = "records.xml";
        const char *xsd_filename = "records.xsd";
        if (validateXml(xml_filename, xsd_filename) != 0) {
            fprintf(stderr, "Error validate.\n");
            return 1;
        }
    } else {
        fprintf(stderr, "Invalid operation.\n");
        return 1;
    }

    return 0;
}
