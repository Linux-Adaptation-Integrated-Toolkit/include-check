extern "C" {
  #include "clang-c/Index.h"
	#include "cjson/cJSON.h"
}
#include <llvm/Support/CommandLine.h>
#include <iostream>

using namespace llvm;

//使用llvm::cl::opt的类模板，创建一个对象CACppFile，用以在全局声明一个新的参数
//Positional指位置参数，即一个路径
static cl::opt<std::string> CACppFile(cl::Positional, cl::desc("<c/cpp file name>"), cl::Required);

//求数组长度
template<typename T> int get_array_num(T &a);

int main(int argc, char** argv) {
	//捕获参数
	cl::ParseCommandLineOptions(argc, argv, "Diagnostics gather");

	//创建一个索引，接受两个bool值参数，第一个参数表示是否从pch中排除，第二个参数表示是否显示诊断
	CXIndex index = clang_createIndex(0, 0);
	const char *args[] = {
		"-I/usr/include",
		"-I.",
		"-E",
		"-Qunused-arguments"
	};
	//从源码解析为编译单元，并返回报错(如果有)
	//CXTranslationUnit_Incomplete表示语义分析将被抑制，重点进行#include展开
	CXTranslationUnit translationUnit;
	CXErrorCode errorCode = clang_parseTranslationUnit2(index, CACppFile.c_str(), args, get_array_num(args), NULL, 0, CXTranslationUnit_Incomplete, &translationUnit);
	//获取编译单元的报错信息数
	unsigned diagnosticCount = clang_getNumDiagnostics(translationUnit);
	//如果编译单元产生报错，收集报错信息，并转为json数据
	if (diagnosticCount > 0 || errorCode != CXError_Success) {
		//创建一个json结构体，存放报错信息
		cJSON* include_check = NULL;
		include_check = cJSON_CreateObject();
		cJSON_AddNumberToObject(include_check, "error_code", errorCode);
		for (unsigned i = 0; i < diagnosticCount; ++i) {
			cJSON* include_check_child = NULL;
			include_check_child = cJSON_CreateObject();

			CXDiagnostic diagnostic = clang_getDiagnostic(translationUnit, i);
			CXString category = clang_getDiagnosticCategoryText(diagnostic);
			CXString message = clang_getDiagnosticSpelling(diagnostic);
			unsigned severity = clang_getDiagnosticSeverity(diagnostic);
			CXSourceLocation loc = clang_getDiagnosticLocation(diagnostic);
			CXString fName;
			unsigned line = 0, col = 0;
			clang_getPresumedLocation(loc, &fName, &line, &col);

			cJSON_AddStringToObject(include_check_child, "file", clang_getCString(fName));
			cJSON_AddNumberToObject(include_check_child, "severity", severity);
			cJSON_AddNumberToObject(include_check_child, "line", line);
			cJSON_AddNumberToObject(include_check_child, "col", col);
			cJSON_AddStringToObject(include_check_child, "category", clang_getCString(category));
			cJSON_AddStringToObject(include_check_child, "message", clang_getCString(message));
			cJSON_AddItemToObject(include_check, std::to_string(i).c_str(), include_check_child);

			clang_disposeString(fName);
			clang_disposeString(message);
			clang_disposeString(category);
			clang_disposeDiagnostic(diagnostic);
		}
		std::cout << cJSON_Print(include_check) << std::endl;
	}	else {
		std::cout << "无报错" << std::endl;
	}

	clang_disposeTranslationUnit(translationUnit);
	clang_disposeIndex(index);
	return 0;
}

template<typename T> int get_array_num(T &a) {
	if (sizeof(a) == 0) {
		return 0;
	}
	int args_num = sizeof(a)/sizeof(a[0]);
	return args_num;
}
