#include "lexer.h"

#include <cctype>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <vector>

static constexpr int DECIMAL = 10;
static constexpr int HEXADECIMAL = 16;
static constexpr int OCTAL = 8;
static constexpr int BINARY = 2;

Lexer::Lexer(std::string_view program, std::filesystem::path source,
      ErrorHandler &errorHandler, uint32_t tabSize)
    : program(program), current(0), source(std::move(source)), tabSize(tabSize),
      errorHandler(errorHandler) {
	if (tabSize == 0) {
		throw std::invalid_argument("Tab size must be greater than 0");
	}
}

Lexer &Lexer::operator=(const Lexer &l) {
	if (this == &l) {
		return *this;
	}
	program = l.program;
	current = l.current;
	source = l.source;
	tabSize = l.tabSize;
	errorHandler = l.errorHandler;
	return *this;
}

std::vector<std::unique_ptr<Token>> Lexer::lex() {
	return evaluate(scan());
}

std::vector<std::unique_ptr<Token>> Lexer::scan() {
	slice();

	std::vector<std::unique_ptr<Token>> tokens;
	while (!slices.empty()) {
		auto &s = slices.front();
		slices.pop();
		std::unique_ptr<Whitespace> whitespace = createWhitespace(s);
		if (whitespace != nullptr) {
			tokens.push_back(std::move(whitespace));
			continue;
		}
		std::unique_ptr<Keyword> keyword = createKeyword(s);
		if (keyword != nullptr) {
			tokens.push_back(std::move(keyword));
			continue;
		}
		std::unique_ptr<Primitive> primitive = createPrimitive(s);
		if (primitive != nullptr) {
			tokens.push_back(std::move(primitive));
			continue;
		}
		std::unique_ptr<Punctuation> punctuation = createPunctuation(s);
		if (punctuation != nullptr) {
			tokens.push_back(std::move(punctuation));
			continue;
		}
		tokens.push_back(createSymbolOrLiteral(s));
	}

	tokens.push_back(std::make_unique<EndOfFile>());

	return tokens;
}

void Lexer::slice() {
	size_t line = 1;
	size_t col = 1;

	while (current < program.size()) {
		while (std::isspace(program[current]) != 0) {
			if (program[current] == '\n') {
				slices.emplace(std::string_view(program).substr(current, 1), source, line,
				      col);
				line++;
				col = 1;
			} else if (program[current] == '\r') {
				if (current + 1 < program.size() && program[current + 1] == '\n') {
					slices.emplace(std::string_view(program).substr(current, 2), source,
					      line, col);
					current++;
					line++;
					col = 1;
				} else {
					slices.emplace(std::string_view(program).substr(current, 1), source,
					      line, col);
					line++;
					col = 1;
				}
			} else if (program[current] == '\t') {
				slices.emplace(std::string_view(program).substr(current, 1), source, line,
				      col);
				col = ((col + tabSize - 1) / tabSize) * tabSize + 1;
			} else {
				if (std::isspace(program[current]) != 0) {
					slices.emplace(std::string_view(program).substr(current, 1), source,
					      line, col);
				}
				col++;
			}
			if (++current >= program.size()) {
				break;
			}
		}
		if (current >= program.size()) {
			break;
		}
		size_t tokenStart = current;
		size_t startCol = col;
		do {
			current++;
			col++;
		} while (current < program.size() && !isSep(current));
		slices.emplace(std::string_view(program).substr(tokenStart, current - tokenStart),
		      source, line, startCol);
	}
}

bool Lexer::isSep(size_t offset) {
	// If c is any of these characters, it is by default a separator
	if (std::isspace(program[offset]) != 0 || program[offset] == '('
	      || program[offset] == ')' || program[offset] == ';' || program[offset] == '{'
	      || program[offset] == '}' || program[offset] == '.' || program[offset] == ','
	      || program[offset] == '+' || program[offset] == '-' || program[offset] == '*'
	      || program[offset] == '/' || program[offset] == '%' || program[offset] == '='
	      || program[offset] == ':' || program[offset] == '!' || program[offset] == '<'
	      || program[offset] == '>' || program[offset] == '&' || program[offset] == '|'
	      || program[offset] == '~' || program[offset] == '^') {
		return true;
	}

	// If c is alnum, it is sep as long as prev is not alnum
	if (std::isalnum(program[offset]) != 0 && std::isalnum(program[offset - 1]) == 0) {
		return true;
	}

	return false;
}

std::unique_ptr<Whitespace> Lexer::createWhitespace(const Slice &s) {
	for (char c : s.contents) {
		if (std::isspace(c) == 0) {
			return nullptr;
		}
	}
	return std::make_unique<Whitespace>(s);
}

std::unique_ptr<Keyword> Lexer::createKeyword(const Slice &s) {
	if (s.contents == "return") {
		return std::make_unique<Keyword>(s, Keyword::Type::RETURN);
	}
	if (s.contents == "let") {
		return std::make_unique<Keyword>(s, Keyword::Type::LET);
	}
	return nullptr;
}

std::unique_ptr<Primitive> Lexer::createPrimitive(const Slice &s) {
	if (s.contents == "i8") {
		return std::make_unique<Primitive>(s, Type::I8);
	}
	if (s.contents == "i16") {
		return std::make_unique<Primitive>(s, Type::I16);
	}
	if (s.contents == "i32") {
		return std::make_unique<Primitive>(s, Type::I32);
	}
	if (s.contents == "i64") {
		return std::make_unique<Primitive>(s, Type::I64);
	}
	if (s.contents == "u8") {
		return std::make_unique<Primitive>(s, Type::U8);
	}
	if (s.contents == "u16") {
		return std::make_unique<Primitive>(s, Type::U16);
	}
	if (s.contents == "u32") {
		return std::make_unique<Primitive>(s, Type::U32);
	}
	if (s.contents == "u64") {
		return std::make_unique<Primitive>(s, Type::U64);
	}
	if (s.contents == "f32") {
		return std::make_unique<Primitive>(s, Type::F32);
	}
	if (s.contents == "f64") {
		return std::make_unique<Primitive>(s, Type::F64);
	}
	if (s.contents == "c8") {
		return std::make_unique<Primitive>(s, Type::C8);
	}
	if (s.contents == "c16") {
		return std::make_unique<Primitive>(s, Type::C16);
	}
	if (s.contents == "c32") {
		return std::make_unique<Primitive>(s, Type::C32);
	}
	if (s.contents == "bool") {
		return std::make_unique<Primitive>(s, Type::BOOL);
	}
	if (s.contents == "()") {
		return std::make_unique<Primitive>(s, Type::UNIT);
	}
	return nullptr;
}

std::unique_ptr<Punctuation> Lexer::createPunctuation(const Slice &s) {
	if (s.contents == "(") {
		return std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen);
	}
	if (s.contents == ")") {
		return std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen);
	}
	if (s.contents == ";") {
		return std::make_unique<Punctuation>(s, Punctuation::Type::Semicolon);
	}
	if (s.contents == "{") {
		return std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace);
	}
	if (s.contents == "}") {
		return std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace);
	}
	if (s.contents == ".") {
		return std::make_unique<Punctuation>(s, Punctuation::Type::Period);
	}
	if (s.contents == ",") {
		return std::make_unique<Punctuation>(s, Punctuation::Type::Comma);
	}
	if (s.contents == "=") {
		return std::make_unique<Punctuation>(s, Punctuation::Type::Equals);
	}
	if (s.contents == ":") {
		return std::make_unique<Punctuation>(s, Punctuation::Type::Colon);
	}
	if (s.contents == "+") {
		return std::make_unique<Punctuation>(s, Punctuation::Type::Plus);
	}
	if (s.contents == "-") {
		return std::make_unique<Punctuation>(s, Punctuation::Type::Hyphen);
	}
	if (s.contents == "*") {
		return std::make_unique<Punctuation>(s, Punctuation::Type::Asterisk);
	}
	if (s.contents == "/") {
		return std::make_unique<Punctuation>(s, Punctuation::Type::ForwardSlash);
	}
	if (s.contents == "%") {
		return std::make_unique<Punctuation>(s, Punctuation::Type::Percent);
	}
	if (s.contents == "!") {
		return std::make_unique<Punctuation>(s, Punctuation::Type::Exclamation);
	}
	if (s.contents == "<") {
		return std::make_unique<Punctuation>(s, Punctuation::Type::LessThan);
	}
	if (s.contents == ">") {
		return std::make_unique<Punctuation>(s, Punctuation::Type::GreaterThan);
	}
	if (s.contents == "&") {
		return std::make_unique<Punctuation>(s, Punctuation::Type::Ampersand);
	}
	if (s.contents == "|") {
		return std::make_unique<Punctuation>(s, Punctuation::Type::VerticalBar);
	}
	if (s.contents == "~") {
		return std::make_unique<Punctuation>(s, Punctuation::Type::Tilde);
	}
	if (s.contents == "^") {
		return std::make_unique<Punctuation>(s, Punctuation::Type::Caret);
	}
	return nullptr;
}

std::unique_ptr<SymbolOrLiteral> Lexer::createSymbolOrLiteral(const Slice &s) {
	return std::make_unique<SymbolOrLiteral>(s);
}

std::vector<std::unique_ptr<Token>> Lexer::evaluate(
      std::vector<std::unique_ptr<Token>> tokens) {
	size_t i = 0;
	std::unique_ptr<Token> token = std::move(tokens[i]);
	std::vector<std::unique_ptr<Token>> evaluated;
	while (!dynamic_cast<EndOfFile *>(token.get())) {
		if (dynamic_cast<Keyword *>(token.get())
		      || dynamic_cast<Primitive *>(token.get())) {
			evaluated.push_back(std::move(token));
		} else if (dynamic_cast<Punctuation *>(token.get())) {
			switch (dynamic_cast<Punctuation *>(token.get())->type) {
				case Punctuation::Type::OpenParen:
				case Punctuation::Type::CloseParen:
				case Punctuation::Type::Semicolon:
				case Punctuation::Type::OpenBrace:
				case Punctuation::Type::CloseBrace:
				case Punctuation::Type::Period: {
					evaluated.push_back(std::move(token));
					break;
				}
				case Punctuation::Type::Comma: {
					evaluated.push_back(
					      std::make_unique<Operator>(token.get(), Operator::Type::Comma));
					break;
				}
				case Punctuation::Type::Equals: {
					Token *next = tokens[i + 1].get();
					if (dynamic_cast<Punctuation *>(next)
					      && dynamic_cast<Punctuation *>(next)->type
					               == Punctuation::Type::Equals) {
						evaluated.push_back(std::make_unique<Operator>(token.get(),
						      Operator::Type::Equality));
						i++;
					} else {
						evaluated.push_back(std::make_unique<Operator>(token.get(),
						      Operator::Type::Assignment));
					}
					break;
				}
				case Punctuation::Type::Colon: {
					Token *next = tokens[i + 1].get();
					if (dynamic_cast<Punctuation *>(next)
					      && dynamic_cast<Punctuation *>(next)->type
					               == Punctuation::Type::Colon) {
						evaluated.push_back(std::make_unique<Operator>(token.get(),
						      Operator::Type::Scope));
						i++;
					} else {
						evaluated.push_back(std::move(token));
					}
					break;
				}
				case Punctuation::Type::Plus: {
					evaluated.push_back(std::make_unique<Operator>(token.get(),
					      Operator::Type::Addition));
					break;
				}
				case Punctuation::Type::Hyphen: {
					evaluated.push_back(std::make_unique<Operator>(token.get(),
					      Operator::Type::Subtraction));
					break;
				}
				case Punctuation::Type::Asterisk: {
					evaluated.push_back(std::make_unique<Operator>(token.get(),
					      Operator::Type::Multiplication));
					break;
				}
				case Punctuation::Type::ForwardSlash: {
					evaluated.push_back(std::make_unique<Operator>(token.get(),
					      Operator::Type::Division));
					break;
				}
				case Punctuation::Type::Percent: {
					evaluated.push_back(std::make_unique<Operator>(token.get(),
					      Operator::Type::Modulus));
					break;
				}
				case Punctuation::Type::Exclamation: {
					Token *next = tokens[i + 1].get();
					if (dynamic_cast<Punctuation *>(next)
					      && dynamic_cast<Punctuation *>(next)->type
					               == Punctuation::Type::Equals) {
						evaluated.push_back(std::make_unique<Operator>(token.get(),
						      Operator::Type::Inequality));
						i++;
					} else {
						evaluated.push_back(std::make_unique<Operator>(token.get(),
						      Operator::Type::LogicalNot));
					}
					break;
				}
				case Punctuation::Type::LessThan: {
					Token *next = tokens[i + 1].get();
					if (dynamic_cast<Punctuation *>(next)
					      && dynamic_cast<Punctuation *>(next)->type
					               == Punctuation::Type::Equals) {
						evaluated.push_back(std::make_unique<Operator>(token.get(),
						      Operator::Type::LessThanOrEqual));
						i++;
					} else if (dynamic_cast<Punctuation *>(next)
					           && dynamic_cast<Punctuation *>(next)->type
					                    == Punctuation::Type::LessThan) {
						evaluated.push_back(std::make_unique<Operator>(token.get(),
						      Operator::Type::BitwiseShiftLeft));
						i++;
					} else {
						evaluated.push_back(std::make_unique<Operator>(token.get(),
						      Operator::Type::LessThan));
					}
					break;
				}
				case Punctuation::Type::GreaterThan: {
					Token *next = tokens[i + 1].get();
					if (dynamic_cast<Punctuation *>(next)
					      && dynamic_cast<Punctuation *>(next)->type
					               == Punctuation::Type::Equals) {
						evaluated.push_back(std::make_unique<Operator>(token.get(),
						      Operator::Type::GreaterThanOrEqual));
						i++;
					} else if (dynamic_cast<Punctuation *>(next)
					           && dynamic_cast<Punctuation *>(next)->type
					                    == Punctuation::Type::GreaterThan) {
						evaluated.push_back(std::make_unique<Operator>(token.get(),
						      Operator::Type::BitwiseShiftRight));
						i++;
					} else {
						evaluated.push_back(std::make_unique<Operator>(token.get(),
						      Operator::Type::GreaterThan));
					}
					break;
				}
				case Punctuation::Type::Ampersand: {
					Token *next = tokens[i + 1].get();
					if (dynamic_cast<Punctuation *>(next)
					      && dynamic_cast<Punctuation *>(next)->type
					               == Punctuation::Type::Ampersand) {
						evaluated.push_back(std::make_unique<Operator>(token.get(),
						      Operator::Type::LogicalAnd));
						i++;
					} else {
						evaluated.push_back(std::make_unique<Operator>(token.get(),
						      Operator::Type::BitwiseAnd));
					}
					break;
				}
				case Punctuation::Type::VerticalBar: {
					Token *next = tokens[i + 1].get();
					if (dynamic_cast<Punctuation *>(next)
					      && dynamic_cast<Punctuation *>(next)->type
					               == Punctuation::Type::VerticalBar) {
						evaluated.push_back(std::make_unique<Operator>(token.get(),
						      Operator::Type::LogicalOr));
						i++;
					} else {
						evaluated.push_back(std::make_unique<Operator>(token.get(),
						      Operator::Type::BitwiseOr));
					}
					break;
				}
				case Punctuation::Type::Tilde: {
					evaluated.push_back(std::make_unique<Operator>(token.get(),
					      Operator::Type::BitwiseNot));
					break;
				}
				case Punctuation::Type::Caret: {
					evaluated.push_back(std::make_unique<Operator>(token.get(),
					      Operator::Type::BitwiseXor));
					break;
				}
			}
		} else if (dynamic_cast<SymbolOrLiteral *>(token.get())) {
			if (std::isdigit(dynamic_cast<SymbolOrLiteral *>(token.get())->s[0]) != 0) {
				std::unique_ptr<Token> literal
				      = evaluateLiteral(dynamic_cast<SymbolOrLiteral *>(token.get()));
				if (literal == nullptr) {
					errorHandler.error(*token, "Invalid integer literal");
					evaluated.push_back(std::move(token));
				} else {
					evaluated.push_back(std::move(literal));
				}
			} else {
				evaluated.push_back(std::make_unique<Symbol>(
				      dynamic_cast<SymbolOrLiteral *>(token.get())));
			}
		} else if (dynamic_cast<Whitespace *>(token.get())) {
			// Ignore whitespace
		} else {
			std::cerr << "Unknown token type\n";
			exit(EXIT_FAILURE);
		}
		i++;
		token = std::move(tokens[i]);
	}
	evaluated.push_back(std::move(token));
	return evaluated;
}

bool Lexer::isDigitInBase(char c, int base) {
	if (base == DECIMAL) {
		return std::isdigit(c) != 0;
	}
	if (base == HEXADECIMAL) {
		return std::isxdigit(c) != 0;
	}
	if (base == OCTAL) {
		return (c >= '0' && c <= '7');
	}
	if (base == BINARY) {
		return (c == '0' || c == '1');
	}
	return false;
}

std::unique_ptr<IntegerLiteral> Lexer::evaluateLiteral(SymbolOrLiteral *literal) {
	int base = DECIMAL;
	size_t start = 0;
	size_t pos = 0;
	for (size_t i = 0; i < literal->s.size(); i++) {
		if (!isDigitInBase(literal->s[i], base)) {
			if (i == 1) {
				if (literal->s[0] == '0') {
					if (literal->s[1] == 'x') {
						base = HEXADECIMAL;
						start = 2;
						continue;
					}
					if (literal->s[1] == 'o') {
						base = OCTAL;
						start = 2;
						continue;
					}
					if (literal->s[1] == 'b') {
						base = BINARY;
						start = 2;
						continue;
					}
				}
			}
			std::string_view suffix = literal->s.substr(i);
			std::string_view value = literal->s.substr(start, i);
			// Inspired by Rust - parse all integers as u128 (here, u64 since no support
			// for 128bit at the moment) and then when we build the AST we will convert to
			// the proper sized type
			// https://doc.rust-lang.org/reference/tokens.html#integer-literals
			// https://doc.rust-lang.org/reference/expressions/literal-expr.html#integer-literal-expressions
			if (suffix == "i8") {
				uint64_t intValue = std::stoul(std::string(value), &pos, base);
				if (pos != i - start) {
					return nullptr;
				}
				return std::make_unique<IntegerLiteral>(literal, Type::I8, intValue);
			}
			if (suffix == "i16") {
				uint64_t intValue = std::stoul(std::string(value), &pos, base);
				if (pos != i - start) {
					return nullptr;
				}
				return std::make_unique<IntegerLiteral>(literal, Type::I16, intValue);
			}
			if (suffix == "i32") {
				uint64_t intValue = std::stoul(std::string(value), &pos, base);
				if (pos != i - start) {
					return nullptr;
				}
				return std::make_unique<IntegerLiteral>(literal, Type::I32, intValue);
			}
			if (suffix == "i64") {
				uint64_t intValue = std::stoul(std::string(value), &pos, base);
				if (pos != i - start) {
					return nullptr;
				}
				return std::make_unique<IntegerLiteral>(literal, Type::I64, intValue);
			}
			if (suffix == "u8") {
				uint64_t intValue = std::stoul(std::string(value), &pos, base);
				if (pos != i - start) {
					return nullptr;
				}
				return std::make_unique<IntegerLiteral>(literal, Type::U8, intValue);
			}
			if (suffix == "u16") {
				uint64_t intValue = std::stoul(std::string(value), &pos, base);
				if (pos != i - start) {
					return nullptr;
				}
				return std::make_unique<IntegerLiteral>(literal, Type::U16, intValue);
			}
			if (suffix == "u32") {
				uint64_t intValue = std::stoul(std::string(value), &pos, base);
				if (pos != i - start) {
					return nullptr;
				}
				return std::make_unique<IntegerLiteral>(literal, Type::U32, intValue);
			}
			if (suffix == "u64") {
				uint64_t intValue = std::stoul(std::string(value), &pos, base);
				if (pos != i - start) {
					return nullptr;
				}
				return std::make_unique<IntegerLiteral>(literal, Type::U64, intValue);
			}
			// Not a suffix, but not a digit, so not a valid literal
			return nullptr;
		}
	}
	// No non-digit character found, treat as regular i32 literal
	std::string_view value = literal->s.substr(start, literal->s.size());
	uint64_t intValue = std::stoul(std::string(value), &pos, base);
	if (pos != literal->s.size() - start) {
		return nullptr;
	}
	return std::make_unique<IntegerLiteral>(literal, Type::I32, intValue);
}
