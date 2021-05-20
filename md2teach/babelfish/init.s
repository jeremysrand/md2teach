;
;  init.s
;  md2teach
;
;  Created by Jeremy Rand on 2021-05-17.
;
;

        mcopy init.macros
		case on
        keep init

init    start
		
        phb
        phk
        plb


		jsl setup
		
		plb
    	
        rtl

		end
