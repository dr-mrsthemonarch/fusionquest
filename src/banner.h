//
// Created by Kyle on 15.06.24.
//

#ifndef BANNER_H
#define BANNER_H

#include <string>
#include <vector>

// Include the text file content
const std::string fileContent = R"(
&&&&&$XXX$XxxX$xx++;+X$$$$$XXXX++++x++++;++xxx+;;;xx;:;+$$;;+X&&xX$$&x+X+x$X;X+:;+$$xx$X++;x$+:;;++++++xXxx++xxX$$$$$$$$&&&$$&&&&&&&&&$X&&$$&&&$$&&$XX
$$&&&$xX$XXxxx;;++xxxXxx++++xx+x+++xxxxx++;:;+x+;:;xX+;+$&$XX;XX+x$$&x+x+x&X;++:;+$X+;+++XXX$X+;xX+;;+XX+++xX$$XxxxxXXXXX&&$$$$X$$$X$$&&&&&X&&&&$&&&$X
$$$$XXXXXXx++;;x+++;;;;+++xXx+++xxxx+;;;+++;;:;+x;:;X$xx$&xXX;+;++X$&X+x++&X;;+::;XX+;;;;x+$X$XxXx++X$x;;+XXxX$$Xx++xX$&$$$$$$&&$$$$X$XX$&&$$&&&&&&&&$
$$Xxxx+x++;;;:::::;+Xx++;;+x+++++++++xx+;;;++;::X&Xx+++X&&$xX;+x+;X&&X+xxx&x;;+:.:Xx;;+;+xxx+$++x+x$$x+;xXxx+xXXXx+;;xX$$$$&&$$$$$&X&&$X$$&&&&&&&$&&$$
XXXx++▐▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▌&&&$X
XXXX+;▐    /\\\\\\\\\\\\\\                                                                                                                      ▌&$&$&
$$$x++▐    \/\\\//////////                                                                                                                      ▌&&$&X
$XX+++▐     \/\\\                                                                    /\\\                                                       ▌&&$&$
$xx++:▐      \/\\\\\\\\\\\               /\\\    /\\\           /\\\\\\\\\\          \///               /\\\\\              /\\/\\\\\\          ▌&&$$&
xxx+::▐       \/\\\///////               \/\\\   \/\\\          \/\\\//////            /\\\            /\\\///\\\           \/\\\////\\\        ▌$$$$&
xx+:::▐        \/\\\                      \/\\\   \/\\\          \/\\\\\\\\\\          \/\\\           /\\\  \//\\\          \/\\\  \//\\\      ▌XX$$$
x+::::▐         \/\\\                      \/\\\   \/\\\          \////////\\\          \/\\\          \//\\\  /\\\           \/\\\   \/\\\     ▌XX$&&
+:::;;▐          \/\\\                      \//\\\\\\\\\            /\\\\\\\\\\          \/\\\           \///\\\\\/            \/\\\   \/\\\    ▌XX$&&
;::;;;▐           \///                        \/////////            \//////////           \///              \/////              \///    \///    ▌&&$$&
;x+;;+▐                /\\\                                                                                                                     ▌&&&$$
X+;+x+▐            /\\\//\////\\\                                                                                    /\\\                       ▌&$$$$
+++xXX▐            /\\\      \//\\\           /\\\    /\\\              /\\\\\\\\            /\\\\\\\\\\           /\\\\\\\\\\\                 ▌XX$&$
xx$$XX▐            \//\\\      /\\\           \/\\\   \/\\\            /\\\/////\\\          \/\\\//////           \////\\\////                 ▌$&&&$
+xX$$&▐              \///\\\\/\\\\/            \/\\\   \/\\\           /\\\\\\\\\\\           \/\\\\\\\\\\             \/\\\                    ▌&&&&&
+X$XX$▐                 \////\\\//              \/\\\   \/\\\          \//\\///////            \////////\\\             \/\\\ /\\               ▌&&&$$
XXXX$$▐                     \///\\\\\\           \//\\\\\\\\\            \//\\\\\\\\\\           /\\\\\\\\\\              \//\\\\\              ▌$&&$$
X$XX$&▐                        \//////             \/////////             \//////////            \//////////               \/////               ▌$$&&$
$XX$$$▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌$$&&$
$XX$&&$x+;:;$&&&&&&$X$+xxx++x;+$xXxxXXXx++++xx;x$x;;;+x++++xxXXXX+;+x+;++Xx++++++xxxXx;::;;;+$&&$XXx$X++$XXxxxX$&&&$&&&&$&&&&&$XXxX$$$&&$X$$$$$$X$$&&$
XXXX$$$x+;:+$&&$&&&$X$xxxxXxx+XxxxxxxxxX$XXXx+;+$x;::;x+;;;+xxXXX+;++;;;+xxx++++++xxXx+:;;;;+$&$$Xxx$$$$$$xxx+xX&&&&&&&&&&&XXxxxXxx$$$&&$$$&$$$$XXX$$$
$X$$$$$x+;:;x$&&&&&&&&Xxx+xxx+x;+++;;:;xx+;x++;x$x;::;xx+++xxXXXX+;++;;:;Xxxxxxx+xxxXx;::;;;+$&$$xxx$&$$&$Xxx++X&&&&&&&&&$&$X&&$Xxx$&$$&$X$&$$$$$XX$$$
$X&&&&$+;;:+X&&&&$&&&&XXXXXXXx+++++;;;++xxxx;;;+Xx;;:;x++++xxXXXX;;++;::;xxxx+++++xxXx;::;;;;xXX$&&&&XxX$$Xxx++X&&&&$&&&&&&$x&&$xxxX$$$$$XX&$$$$Xxx$$$
XX$&&$$x+;;x$&$$&&&&&$xXXXX$X;+xxx++;++++xxx;++xX+;::;xx+++xxXXXX;++++;;;XXxxxxx++xxXx;:;;;;;xXX$$$&$x++X$Xxx++X&&&&&&&&&&&Xx&&$+++X$$$$$XX&$$$$XxX$$$
XX$&&&$x+;;+$$$&&&&$&&$X;.+Xx++x++x+++++;+x+;x;+X+;::;x++;;xxXXXXxxxx++++Xxxxx+x++xxXx+:;;;;+xXXX$&&&x++X$Xxx++X&&&&&&&&&&&xx$$X++xX$$$X$XX$$$&$XxX$$$
$$$&&&&X+;;;;X$$$&&$&&$x+;;xX+xxxxx+++xxxxx+;XxxX;;::+XxxxxXX$$$X++x+;;;;XXXXxxxxxXX$X+;;;;;+xXXX$&&&x++x$Xxx++X&&&&&&&&&&&xX$$x++x$$$$X$x$&$$&$xxX$X$
$$$$$&&Xx+;:+X&&$X&$$&$X+;+xxxxxxXx++xxXXXX+;XxXX;;::+Xxxx+xXXX$X++x++;;;XXxxxxx+xxxXX+;;;;;+xX$$$&&&X+xx$$x+++$&&&&&&&&&&$+$$X+++X&&$$XXx$$$$$X+xX$$$
XX$x+X&$X+;:+$&&&&&$$&&$x;+++++xxxxxxXX$$$$++$xxx;;::xXx+++xXXXXx;+++;;;;XXXXXXXxXXX$$+;;;+;+x&&&$$$&X+xx$$X+++$&&&&&&&&&&xxXXx++x$$$$X$x$$&$&$xxX$$$$
$$&&&&&$XX+;;+X$$&&&$&$X+++xX;:;X$XXX$XXxx+;+X+xx;;:;x$xx++xXXXXx++x++;;;XXxxxxxxxxXX$+;;++++x&&$$X$&$x+x$$x+++X&&&&&&&&&$X$$Xx++X$$$$$$X&&$&&$xx$&$$$
$&$$&&XX$Xx;:;+$&&&&&&&$xXX++x::+$$$$&&&&$x;x$xxx;;:;X$xxxxxXXXXx++xx++++$XXXXXx+xxxX$+;;;;;+x&&&$X$&&$X$&$x++;x&&&&&&&&&&&&&&$xX$&&&$$X&&$$&$xxX$$$$$
$&$$$$xxxX$x;:+&&$&&&$$Xx$&X$&+::xXXxxX$Xx+;XXxX;;;;+X$xxxxXXXXXx++x++;;+XXXXxxx+++xX$x;;;;++x$&&$$$&&&$$X&X+;;;XX$&$$&&&&&&&&XX$$$$$&$&&&&&&XX$$X$$$X
X$$Xxxxxxx$&x;:+xX$$$X$$$$$xX$&+:::;;+xXXx++X$&X;;;;+$$XxxxXXXXXx++x++;;+XXXxxxXxxXXX&x;;;;;++$&&$X$&&Xxxx$$+;::+$$$X$$$xX$&$xx&&&&&&$$$&&&&$X$$XX$$XX
XxXxxxxxxxX$$x;:++xXXXx+XXX+::X$x++++x+;+xxxXXXx;;;;x$$XxxxXXXXXx++x+;;;+XXxx+xxxxxxX&X+;;;;++X&&$$$&&$xxxX$x::.;xXX$$$XXxXX+x$&$&&&&&&&&$$$X$$X$$$XXX
xXxxxxxxx+xx$$x::;;+++XX++x$xXXx;;;+x+xxxX$&$XX+;;;;X$$XxxxXXXX$XXXXx+++x$XXXXXXXXXX$&X+;;;;++X&&$$$&&&XX$&&$;...;xXXxXX$$XXX&$XXXx$$&&&&&$XXX$$$$$X$$
+Xxxx+xx+xxxX&$x;::++++X$$X+;;;+xxx++xXxxX$&$XXxxx++X$$$XxxXXXXXxxxx+;;;x$XXXxxXxxXX$$$+;;;;++x&&$$$&&&&&$&&$x....:;+xxxxxxXX$&$xx$$XxX&&$X$&&$$&$X$$$
x+Xxxx+xxxXxxX$$X;::+X$X+;;;+XX+;+xXX$$xXX$&$Xxxxx$&&$$$XxxXXX$X;;:;::::+$XXXxxXXXX$$$$+;;+xXX$&&$$&&&$$$&&&&X;....:;;;;;+xXXXX$$&$xx$$$XX$$&&&X$X$&$$
+x+XXxxxXX$$xX+x$Xx+xX+++;xX+;+xXXXX$$XX$X$$$+x+xxxX$$$$$XXXXX$XXxx+++++xXXXxxxXxXXX$$&&$$&&&&$XXX$$$$X$X&&&&$x:....::::;;;+xXxxxX&&&XXxX$&&&&&$X$&&$$
X+++xX+xX$$xX++++XXXx+++x$x+xXXXX$$$XXX$$$&Xx++xXXxxxx+++++;;;++xX&&&&&$$&XXXXXxxxxxX$$$$$$XXxxXXXXX$&XXXX$&&$X;......::;:;;;xXxxxx$$XX$$&&&&&$X$$$&&&
$X+++xX++X&&$XX&&$X++++xXx+xXXX$$XxxxXXX$&&Xxx+XX+++x+xxxx+x;++++xxxXXXXXXx+x+++;;;++x++x+xxxXXXxXxXX$$XXX$&&&X;........::;;;+xXXxxx$$$&&&$&&$X$$&&&&&
&&$+++xx+++X&&&&&$++++xXx+xXX$$XxxXXXXX$$&&X+xx$$x+x+xXxxXx+;+X+;+xXX++++;++;++;+++xx+xx++XXXX$$xXXXX&&X$X$&&$+:....:....:::;+xXXXx++$$&&&&$X$&$$&&&&$
$&&$x+xxxXXX$$&&&x++;+XX++XX$$xxxX$XXxxxx&&x++x$+++++xX++x::+++++XxxX++x++xx;;;;;+;+x+;+x+x+XX$$xxXXX$&XX$$&$+:....::.....::;++xXXxxxX&&&$X$$&&$$&&&$$
XX$$xXx+++XXX&&&&++++xXx++XX$xxxX$XXxx++++$x++XX+++++Xx++xX+++;++Xx+x++++;+x;;;+:..:x+++++XXXXX$Xx+xXX&$XXXx;.....:::::...::;x+xXXxx+x$$xX$&&&&XX&&$$&
&XX$Xxxx+x++x$&&$x++xxXXxxX$XxxxX$XXx++++++xX+&&X+x++X$$X+;;+x+++Xxxx+++++X$;;;+;;;;xx++x+++x$$&$xXXXX&&Xx;:.....;;;;;:...::;xxxXXX+;xx$$&&&&&&$$$$$&&
$X$&Xx++xx++++xxXx++x+xx++xXxxxxxXXxXx+++++++xx$$+++x$Xxx+++x+++xXx+x++++++x;;;+:..:xx+++++xxXX$$xxxX$$X+:.....;++++;;:...:;;xxXXXXxxX$&&&&&&&$X$$$&&$
&&$XX$$+x$$x+++++x;+x++XX+xXxxxxxxXxXXx+;;+++;+++xXXX&&&&X++x;++xXxxx++x++xX+;;+;::;xx+++x+xX$&&&X$$Xx+;:::::;++x++++;...:;++xxXXx++X$$&&$$$$$X$&&$&$$
&&&&&XX$X$$XXx+++;++;++xXx+xXXxxxxxxxxxxx+;;;+++;++;+xX$&&$++X++$$Xxx++++++x+;;+;;+xXX++xx+&&&&$Xxxx+++++;;+xxXXXx++;:..:+++xxXXXx++X$&&$$$$$XX$$$$$$X
&&&&&&&$XXXxX$xXx++x+xxxxxx++Xx+++xxxxxxxXx+;;;;+++;;;;;++++x$X+$&&&$xxXxX&&+xxxx$&&&$x$$x+xx+++++++xx+++xxX$$$$Xx+;:..:+xx+xXXXX++x$&&&$$XXXX$$$$X$xx
&&&&&&&&&$$XxxXXxxxx++;++xXx;+Xxx++++x+xxxxxxx++;;;+++++;;;;;;;;;+;+++++++Xx++xx++;++;;;+;;;;+++++++++xxX$$$$$$Xx+;;::;+Xx+X$XXxx+x$$&$XxXxx+XXXXXX++x
&&&&&&&&&&&$$XX$xx+X$Xx++++xx;;xx++++++++xXxxxxx+x+;;;++;+++++;;+;;;;;;;::x::;;+;;:;+;;;;+;;;+++;;+xxX$$$X$$$Xxx;;;:::+Xxx$XxXx;;+$&XXXXX$x+x::;xx;;xx
&&&&&&&&&$&$X&$$$$XX$$+;;;;;+x+;;++++++++++xxXxxxxxxxx++;;;;;;;;+;+++;;;;;x;;;;+;;;;;;;;;;::;;+xxXXXX$XX$$$Xx+;;;:::;xx+XXXxx+;;x$&$$$&$X+xX+::;+;;xxx
&&&$x$&&&&&&$X$XX&$&&&X++xxxx+xx+;;++;;;++;;++xXxxxxxxXxxxx++;;;;;;:;:::;;x;;:::;;::;;;;;+++xxXXXXXXXXX$Xx+++;;;;;;+++X$xxxx+;+X$$&&$$&X+x$$Xx+x;;xxx+)";

#endif //BANNER_H
