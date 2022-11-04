// ------------------------------------------------------------------------------
// OVERVIEW
// - Parse glcorearb.h and generate wrappers for GL calls.
// - See render_commands.h and render_command_list.h.
//
// CAUTION: This script generates several ill-formed functions.
//          Actual files in the engine are manually fixed, so
//          compare them with auto-generated files.
// ------------------------------------------------------------------------------

// #todo: Ignore certain old functions? (e.g., glActiveTexture)

use std::io::{Write, BufRead, BufReader};
use std::fs::File;

// Should exist in the same directory.
const INPUT_GLCOREARB: &str = "glcorearb.h";
// Contains render packets (wrappers for GL device calls).
const OUTPUT_RENDER_PACKET: &str = "render_commands.generated.h";
// Contains render commands that create render packets.
const OUTPUT_RENDER_COMMAND_LIST: &str = "render_command_list.generated.h";

// All GL function signatures are in-between these markers.
const GLAPI_BEGIN_MARKER: &str = "#ifdef GL_GLEXT_PROTOTYPES";
const GLAPI_END_MARKER: &str = "#endif";

// Represents a GL function signature.
struct GLFnInfo {
	return_type: String,
	gl_func_name: String,
	func_name: String,
	params: Vec<String>,
	param_names: Vec<String>
}

fn main() {
	// ------------------------------------------------------------------------------
	// Parse the input file
	// ------------------------------------------------------------------------------

	let input_file = File::open(INPUT_GLCOREARB).unwrap();
	let reader = BufReader::new(input_file);

	let mut function_db: Vec<GLFnInfo> = Vec::new();

	println!("Input: {}", INPUT_GLCOREARB);

	let mut is_parsing_api = false;
	for (_, line) in reader.lines().enumerate() {
		let line = line.unwrap();
		if is_parsing_api == false {
			if line == GLAPI_BEGIN_MARKER {
				is_parsing_api = true;
			}
		} else {
			if line == GLAPI_END_MARKER {
				is_parsing_api = false;
			} else {
				let mut parse_result = GLFnInfo {
					return_type:  String::new(),
					gl_func_name: String::new(),
					func_name:    String::new(),
					params:       Vec::new(),
					param_names:  Vec::new()
				};
				parse_api(line, &mut parse_result);
				function_db.push(parse_result);
			}
		}
	}

	println!("GL functions parsed: {}", function_db.len());

	// ------------------------------------------------------------------------------
	// render_commands.generated.h
	// ------------------------------------------------------------------------------

	let mut out_file = File::create(OUTPUT_RENDER_PACKET).unwrap();

	// Write each render packet.
	for fn_info in &function_db {
		let mut var_decls = String::new();
		for param in fn_info.params.iter() {
			if param != &"void" {
				var_decls.push('\t');
				var_decls.push_str(param);
				var_decls.push(';');
				var_decls.push('\n');
			}
		}
	
		let mut gl_call_arguments = String::new();
		for (ix, param) in fn_info.param_names.iter().enumerate() {
			gl_call_arguments.push_str("\t\t\t");
			gl_call_arguments.push_str(&format!("params->{}", param));
			if ix != fn_info.param_names.len() - 1 {
				gl_call_arguments.push_str(",\n");
			} else {
				gl_call_arguments.push_str("\n");
			}
		}

		out_file.write_fmt(format_args!(
r"struct RenderCommand_{} : public RenderCommandBase {{
{}
	static void APIENTRY execute(const RenderCommand_{}* __restrict params) {{
		{}(
{}		);
	}}
}};
",
			fn_info.func_name,
			var_decls,
			fn_info.func_name,
			fn_info.gl_func_name,
			gl_call_arguments)).unwrap();
	}

	// Write the union packet.
	let mut union_members = String::new();
	for fn_info in &function_db {
		union_members.push_str(&format!("\tRenderCommand_{} {};\n", fn_info.func_name, fn_info.func_name));
	}

	out_file.write_fmt(format_args!(
r"union RenderCommandPacketUnion {{
public:
	PFN_EXECUTE execute;
private:
{}
}};
", union_members)).unwrap();

	out_file.sync_all().unwrap();

	println!("Generated: {}", OUTPUT_RENDER_PACKET);

	// ------------------------------------------------------------------------------
	// render_command_list.generated.h
	// ------------------------------------------------------------------------------
	let mut cmd_list_file = File::create(OUTPUT_RENDER_COMMAND_LIST).unwrap();

	for fn_info in function_db {
		let mut fn_args = String::new();
		for (ix, param) in fn_info.params.iter().enumerate() {
			fn_args.push_str(&format!("\t{}", param));
			if ix != fn_info.params.len() - 1 {
				fn_args.push_str(",\n");
			}
		}

		let mut assign_args = String::new();
		for (_, param) in fn_info.param_names.iter().enumerate() {
			assign_args.push_str(&format!("\tpacket->{} = {};\n", param, param));
		}

		cmd_list_file.write_fmt(format_args!(
r"{} {}(
{})
{{
	RenderCommand_{}* __restrict packet = (RenderCommand_{}*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_{}::execute);
{}}}
",
		fn_info.return_type,
		fn_info.func_name,
		fn_args,
		fn_info.func_name,
		fn_info.func_name,
		fn_info.func_name,
		assign_args)).unwrap();
	}

	cmd_list_file.sync_all().unwrap();

	println!("Generated: {}", OUTPUT_RENDER_COMMAND_LIST);
}

// <Example>
// line = "GLAPI void APIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const void *indices);"
// parse_result = GLFnInfo {
//     return_type  : "void",
//     gl_func_name : "glDrawElements",
//     func_name    : "drawElements",
//     params       : ["GLenum mode", "GLsizei count", "GLenum type", "const void *indices"],
//     param_names  : ["mode", "count", "type", "indices"]
// }
fn parse_api(line: String, parse_result: &mut GLFnInfo) {
	// GLAPI <return_type> APIENTRY
	let mut tokens = line.split(" ");
	let _ = tokens.next().unwrap();
	let mut return_type = tokens.next().unwrap().to_string();
	let apientry = tokens.next().unwrap();
	if apientry.find('*').is_some() {
		return_type.push_str("*");
	}

	let gl_func_name_begin = line.find("gl").unwrap();
	let gl_func = &line[gl_func_name_begin..];

	let paren_begin_index = gl_func.find("(").unwrap();
	let paren_end_index = gl_func.find(")").unwrap();

	// GLAPI <return_type> APIENTRY gl_func_name(...)
	let gl_func_name = &gl_func[0..(paren_begin_index-1)];
	let mut func_name;
	let func_first_char = (&gl_func_name[2..]).chars().next().unwrap();
	func_name = func_first_char.to_lowercase().to_string();
	func_name.push_str(&gl_func_name[3..]);

	// params = ["type1 var1", "type2 var2", "type3 var3", ...]
	let gl_params = &gl_func[(paren_begin_index+1)..paren_end_index];
	let params_raw: Vec<&str> = gl_params.split(",").collect();
	let mut params: Vec<&str> = Vec::new();
	let mut param_names: Vec<&str> = Vec::new();
	for param_ in params_raw {
		let param = param_.trim();
		params.push(param);
		match param.rfind(" ") {
			Some(name_begin) => {
				let param_name_star = &param[name_begin+1..];
				match param_name_star.rfind("*") {
					Some(last_star) => param_names.push(&param_name_star[last_star+1..]),
					None => param_names.push(&param_name_star)
				}
			},
			None => (), // probably just 'void'
		}
	}

	// Write GLFnInfo
	parse_result.return_type = String::from(return_type);
	parse_result.gl_func_name = String::from(gl_func_name);
	parse_result.func_name = String::from(func_name);
	for param in params {
		parse_result.params.push(String::from(param));
	}
	for param in param_names {
		parse_result.param_names.push(String::from(param));
	}
}
