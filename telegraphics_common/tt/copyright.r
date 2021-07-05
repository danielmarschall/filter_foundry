#include "systypes.r"

resource'vers'(1){
	0x1,
	0x00,
	beta,
	1,
	verAustralia,
	"1.0b1",
	"1.0b1 ("$$format("%d/%d/%d",$$day,$$month,$$year)")\nby Toby Thain"
};

resource'vers'(2){
	0x1,
	0x00,
	beta,
	1,
	verAustralia,
	"1.0b1",
	"© "$$format("%d",$$year)" Telegraphics Pty Ltd"
};
