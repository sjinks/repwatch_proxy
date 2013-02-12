#include <security/pam_appl.h>

int main(int, char**)
{
	pam_handle_t** h;
	pam_conv c;
	c.conv        = 0;
	c.appdata_ptr = 0;

	pam_start("test", 0, &c, 0);
	return 0;
}
