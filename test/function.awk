#
# FIX ME please !!
#
# - bushi@mizi.com
#

BEGIN { 
	VIVI_MAP = "../vivi.map"
	FUNCTION_MAP = "function.map"
	name_of_function =  "" 
	prototype = ""
	print("/* ") > FUNCTION_MAP
	print(" * Auto generated from function.list by function.awk") \
		>> FUNCTION_MAP
	print(" *") >> FUNCTION_MAP
	print(" * - bushi@mizi.com") >> FUNCTION_MAP
	print(" */") >> FUNCTION_MAP
	print("") >> FUNCTION_MAP
}

{
	prototype = $0
	name_of_function = \
		gensub(/(.*) (.*)\(\*(.*)\)\((.*)\)/, "\\3", "", prototype)
	grep_cmd = "grep \" "name_of_function"$\" "VIVI_MAP
	pipe_cmd1 = "|cut -d ' ' -f-1"
	pipe_cmd2 = "| awk '{print\""prototype" = (void*)(0x\"\$0\");\"}'"
	direct_cmd = ">> "FUNCTION_MAP
	system(grep_cmd pipe_cmd1 pipe_cmd2 direct_cmd)
}

END {
}
