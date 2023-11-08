import os;
import json;
import re;
import sys;

REFLECT_FILE_NAME = "generated_reflect"
REFLECT_CMAKE_NAME = "greflect"

class GPropertyExtents:
    def __init__(self):
        self.name = None;
        self.getter = None;
        self.setter = None;
        

class GFileAttrib:
    def __init__(self,file_path,file_class_attribs):
        self.file_path = file_path;
        self.class_attribs = file_class_attribs;

class GClassAttrib:
     def __init__(self,class_struct_type,method_attribs,prop_attribs):
        self.class_struct_type = class_struct_type;
        self.gmethod_attribs = method_attribs;
        self.prop_attribs = prop_attribs;

class GPropertyAttrib:
     def __init__(self,prop_name,prop_ref,owner_name,full_prop):
        self.prop_name = prop_name;
        self.prop_ref = prop_ref;
        self.owner_name = owner_name;
        self.full_prop = full_prop;
        self.getter = None;
        self.setter = None;

class GMethodAttrib:
    def __init__(self,method_name,owner_name,full_method):
        self.method_name = method_name;
        self.owner_name = owner_name;
        self.full_method = full_method;

class ClassStructType:
    def __init__(self,struct_type,class_struct_name,begin_index,end_index):
        self.struct_type = struct_type;
        self.class_struct_name = class_struct_name;
        self.begin_index= begin_index;
        self.end_index = end_index;


def find_class_end(file_content, start_pos):
    balance = 0
    i = start_pos
    while i < len(file_content):
        if file_content[i] == '{':
            balance += 1
        elif file_content[i] == '}':
            balance -= 1
            if balance == 0:
                return i + 1
        i += 1
    return -1

def find_class_or_struct_name_and_type(file_content):
    class_struct_pattern = r'(class|struct)\s+(\w+)\s*(?::\s*\w+\s*)?(?::{)?'
    class_struct_matches = re.finditer(class_struct_pattern, file_content)
    class_struct_objs=[];
    for class_struct_match in class_struct_matches:
        class_struct_type = class_struct_match.group(1)
        class_struct_name = class_struct_match.group(2)
        class_start_pos = class_struct_match.start();
        class_end_pos = find_class_end(file_content,class_start_pos)
        if(class_end_pos != -1):
            class_struct_objs.append(ClassStructType(class_struct_type,class_struct_name,class_start_pos,class_end_pos))

    return class_struct_objs;

def check_for_prop_attrib(prop_def):
    attribs = prop_def.split(',')
    extents = GPropertyExtents()
    for attrib in attribs:
        prop_value = attrib.split('=')
        if prop_value.__len__() != 2:
            continue
        stripped_attrib = prop_value[0].strip()
        stripped_val = prop_value[1].strip()
        if stripped_attrib == 'NAME':
            extents.name = stripped_val;
        elif stripped_attrib == 'GETTER':
            extents.getter = stripped_val;
        elif stripped_attrib == 'SETTER':
            extents.setter = stripped_val;
    return extents


def read_attribs_in_h(filepath,attribs):
    with open(filepath,"r") as read_file:
        file_content = read_file.read()
       

        class_struct_types = find_class_or_struct_name_and_type(file_content);

        gmethod_pattern = r'GMETHOD\(\)\s*[^;]*\n\s*([^;]+\([^\)]*\))'
        gprop_pattern = r'GPROPERTY\(([^)]*)\)\s*([^;]+);'


        file_class_attribs = []
        any_added_in_file = False;
        for class_struct_type in class_struct_types:
            file_attrib_and_val = [];
            file_prop_attrib_and_val = [];
            any_added = False;
            matches = re.finditer(gmethod_pattern, file_content[class_struct_type.begin_index:class_struct_type.end_index])
            print(f"Checking file : {filepath}")
            for match in matches:
                any_added = True
                c_method_definition = match.group(1)
                first_blank = c_method_definition.index(' ');
                first_paranth = c_method_definition.index('(');
                c_method_name = c_method_definition[first_blank+1:first_paranth]
                file_attrib_and_val.append(GMethodAttrib(c_method_name,class_struct_type.class_struct_name,c_method_definition))
            
            matches2 = re.finditer(gprop_pattern, file_content[class_struct_type.begin_index:class_struct_type.end_index])
            for match in matches2:
                any_added = True
                c_prop_definition = match.group(2)
                first_blank = c_prop_definition.index(' ');
                c_prop_def = c_prop_definition[first_blank+1:]
                c_prop_name =  "";
                if c_prop_def.find(' ') != -1:
                    c_prop_name = c_prop_def[0:c_prop_def.index(' ')];
                elif c_prop_def.find('=') != -1:
                    c_prop_name = c_prop_def[0:c_prop_def.index('=')];
                
                extents = check_for_prop_attrib(match.group(1));
                prop_ref = c_prop_name
                if extents.name:
                    c_prop_name = extents.name
                prop_attrib = GPropertyAttrib(c_prop_name,prop_ref,class_struct_type.class_struct_name,c_prop_definition)
                prop_attrib.setter = extents.setter
                prop_attrib.getter = extents.getter
                file_prop_attrib_and_val.append(prop_attrib)
            if any_added:
                any_added_in_file = any_added
                file_class_attribs.append(GClassAttrib(class_struct_type,file_attrib_and_val,file_prop_attrib_and_val))
        if any_added_in_file:
            attribs.append(GFileAttrib(filepath,file_class_attribs));


def iterate_files_in_dir(dir,attribs):
    for filename in os.listdir(dir):
        f = os.path.join(dir,filename);
        if os.path.isfile(f):
            if f.endswith(".h"):
                read_attribs_in_h(f,attribs)
        else:
            iterate_files_in_dir(f,attribs)

def add_include(file_handle,path):
    file_handle.write(f"#include \"{path}\"\n")

def add_prop_reflect_for_class(write_file,class_name,prop_name,prop_ref):
    write_file.write(f"\tGOBJECT_DEFINE_PROPERTY(\"{prop_name}\",&{class_name}::{prop_ref})\n")


def add_prop_reflect_for_class_gs(write_file,class_name,prop_name,prop_ref,getter,setter):
    write_file.write(f"\tGOBJECT_DEFINE_PROPERTY_GS(\"{prop_name}\",&{class_name}::{prop_ref},&{class_name}::{getter},&{class_name}::{setter})\n")


def begin_reflect_for_class(write_file,class_name):
    write_file.write(f"GOBJECT_ENABLE({class_name})\n")

def end_reflect_for_class(write_file):
        write_file.write("}\n")

def generate_local_cmake(reflect_cpp_path,project_cmake_path):
    if os.path.exists(f"{project_cmake_path}/{REFLECT_CMAKE_NAME}.cmake"):
        os.remove(f"{project_cmake_path}/{REFLECT_CMAKE_NAME}.cmake")
    with open(f"{project_cmake_path}/{REFLECT_CMAKE_NAME}.cmake","w") as write_file:
        include_path = reflect_cpp_path.replace("\\", "/");
        write_file.write(f"set(GREFLECT_GENERATED_CPP \"{include_path}\")")

def generate_reflect_cpp(gobject_path,attribs,base_path):
    if os.path.exists(f"{base_path}/{REFLECT_FILE_NAME}.cpp"):
        os.remove(f"{base_path}/{REFLECT_FILE_NAME}.cpp")
    with open(f"{base_path}/{REFLECT_FILE_NAME}.cpp", "w") as write_file:
        add_include(write_file,gobject_path)
        # First add all class refs
        for attrib in attribs:
            add_include(write_file,attrib.file_path)
        write_file.write("\n\n\n")
        for attrib in attribs:
            file_name = attrib.file_path
            for class_attrib in attrib.class_attribs:
                class_struct_type = class_attrib.class_struct_type;
                class_name = class_struct_type.class_struct_name;
                begin_reflect_for_class(write_file,class_name)
                for prop_attrib in class_attrib.prop_attribs:
                    if prop_attrib.getter and prop_attrib.setter:
                        add_prop_reflect_for_class_gs(write_file,class_name,prop_attrib.prop_name,prop_attrib.prop_ref,prop_attrib.getter,prop_attrib.setter)
                    else:
                        add_prop_reflect_for_class(write_file,class_name,prop_attrib.prop_name,prop_attrib.prop_ref);

                end_reflect_for_class(write_file)
    return os.path.abspath(f"{base_path}/{REFLECT_FILE_NAME}.cpp")

def main():
    config_path = sys.argv[1]
    abs_path = os.path.abspath(f"{config_path}/reflect_config.json")
    print(f"Trying to find reflect_config.json {abs_path}") 
    if os.path.exists(f"{config_path}/reflect_config.json"):
        print("Found") 
        with open(f"{config_path}/reflect_config.json", "r") as read_file:
            data = json.load(read_file)
            gobject_utils_path = data["gobject_utils_path"]
            project_cmake_path = data["project_cmake_path"]
            attribs = [];
            for i in data["folders"]:
                iterate_files_in_dir(i,attribs);
            file_path = generate_reflect_cpp(gobject_utils_path,attribs,project_cmake_path)
            print(f"Generated to here : {file_path} Attrib count : {attribs.__len__()}")
            generate_local_cmake(file_path,project_cmake_path)
    else:
        print("Couldn't find config file")
        
            

if __name__=="__main__": 
    main() 
