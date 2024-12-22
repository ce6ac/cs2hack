#include "memory.h"
#include <cmath>
#include <thread>

uint64_t memory::get_proc_baseaddr()
{
	return proc.baseaddr;
}

process_status memory::get_proc_status()
{
	return status;
}

void memory::check_proc()
{
	if (status == process_status::FOUND_READY)
	{
		short c;
		read<short>(proc.baseaddr, c);

		if (c != 0x5A4D)
		{
			status = process_status::FOUND_NO_ACCESS;
			close_proc();
		}
	}
}

void memory::open_proc(const char* name)
{
	if(!conn)
	{
		ConnectorInventory *inv = inventory_scan();
		conn = inventory_create_connector(inv, "qemu_procfs", "");
		inventory_free(inv);
	}

	if (conn)
	{
		if(!kernel)
		{
			kernel = kernel_build(conn);
		}

		if(kernel)
		{
			Kernel *tmp_ker = kernel_clone(kernel);
			proc.hProcess = kernel_into_process(tmp_ker, name);
		}
		
		if (proc.hProcess)
		{
			Win32ModuleInfo *module = process_module_info(proc.hProcess, name);

			if (module)
			{
				OsProcessModuleInfoObj *obj = module_info_trait(module);
				proc.baseaddr = os_process_module_base(obj);
				os_process_module_free(obj);
				mem = process_virt_mem(proc.hProcess);
				status = process_status::FOUND_READY;
			}
			else
			{
				status = process_status::FOUND_NO_ACCESS;
				close_proc();
			}
		}
		else
		{
			status = process_status::NOT_FOUND;
		}
	}
	else
	{
		printf("memflow: cant create connector\n");
		exit(0);
	}
}

void memory::close_proc()
{
	if (proc.hProcess)
	{
		process_free(proc.hProcess);
		virt_free(mem);	
	}

	proc.hProcess = 0;
	proc.baseaddr = 0;
	mem = 0;
}

uint64_t memory::get_module_address(const char* proc_name, const char* module_name){
	Kernel *tmp_kernel = kernel_clone(kernel);
	auto cs2_process = kernel_into_process(tmp_kernel, proc_name);
	auto info = process_module_info( cs2_process , module_name);
	auto mod_info = module_info_trait(info);
		uint64_t module_base_addr = os_process_module_base(mod_info);
	return module_base_addr;
}
