BEGIN {
	inside_inlined_code_block = 0
	dne = 0  
}

{
	# currently only support c

	# if the code segment has a "DNE"
	# then DO NOT EXECUTE the code block
	if ($0 ~ /^```c DNE/)
	{
		# set the DNE flag
		dne = 1 ;

		# remove the DNE text from $0
		gsub(/DNE/,"", $0)
	}

	print $0

	if ($0 ~ /^```c/) {
		if( dne != 1 )
		{
			inside_inlined_code_block = 1
			close("inline_exec_tmp.c")
			system("rm inline_exec_tmp.c")
		}
		else
		{
			inside_inlined_code_block = 0
		}
	} else if ($0 ~ /^```[:space:]*$/) {
		# terminating a code block?
		# lets run the code, and insert its output
		if (inside_inlined_code_block == 1) {
			print "\nProgram output:\n```"
			system("sh tools/run_code_sample.sh output_tmp.dat")
			while ((getline line < "output_tmp.dat") > 0)
				print line
			close("output_tmp.dat")
			system("rm output_tmp.dat")
			print "```"
			inside_inlined_code_block = 0
		}
	} else if (inside_inlined_code_block == 1) {
		print $0 >> "inline_exec_tmp.c"
	}
}

END {
	system("make --no-print-directory inline_exec_clean")
}
