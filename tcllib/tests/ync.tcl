# Copyright (c) 1996  by Sanjay Ghemawat
switch -exact -- [yes_no_cancel "" "Yes or No?" Yeah!!! Nope. {Forget it}] {
    yes		{error_notify "" "Yes!!!" Info}
    no		{error_notify "" "Nah..." Info}
    cancel	{error_notify "" "What??" Info}
}
