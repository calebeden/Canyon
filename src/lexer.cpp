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
      ErrorHandler *errorHandler, uint32_t tabSize)
    : program(program), source(std::move(source)), tabSize(tabSize),
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
	line = l.line;
	col = l.col;
	return *this;
}

std::vector<std::unique_ptr<Token>> Lexer::lex() {
	return evaluate(scan());
}

std::vector<std::unique_ptr<Token>> Lexer::scan() {
	slice();

	std::vector<std::unique_ptr<Token>> tokens;
	while (!slices.empty()) {
		auto s = slices.front();
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
		std::unique_ptr<Punctuation> punctuation = createPunctuation(s);
		if (punctuation != nullptr) {
			tokens.push_back(std::move(punctuation));
			continue;
		}
		tokens.push_back(createSymbolOrLiteral(s));
	}

	tokens.push_back(std::make_unique<EndOfFile>(Slice("", source, line, col)));

	return tokens;
}

void Lexer::slice() {
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
		if (program[current] == '\'') {
			do {
				if (program[current] == '\\') {
					current++;
					col++;
				}
				current++;
				col++;
			} while (current < program.size() && program[current] != '\'');
			if (current >= program.size()) {
				errorHandler->error(Slice(std::string_view(program).substr(tokenStart,
				                                current - tokenStart),
				                          source, line, startCol),
				      "Unterminated character literal");
				break;
			}
			current++;
			col++;
		} else {
			do {
				current++;
				col++;
			} while (current < program.size() && !isSep(current));
		}
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
	      || program[offset] == '~' || program[offset] == '^'
	      || program[offset] == '\'') {
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
	if (s.contents == "fun") {
		return std::make_unique<Keyword>(s, Keyword::Type::FUN);
	}
	if (s.contents == "if") {
		return std::make_unique<Keyword>(s, Keyword::Type::IF);
	}
	if (s.contents == "else") {
		return std::make_unique<Keyword>(s, Keyword::Type::ELSE);
	}
	if (s.contents == "while") {
		return std::make_unique<Keyword>(s, Keyword::Type::WHILE);
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
		if (dynamic_cast<Keyword *>(token.get())) {
			evaluated.push_back(std::move(token));
		} else if (dynamic_cast<Punctuation *>(token.get())) {
			switch (dynamic_cast<Punctuation *>(token.get())->type) {
				case Punctuation::Type::OpenParen:
				case Punctuation::Type::CloseParen:
				case Punctuation::Type::Semicolon:
				case Punctuation::Type::OpenBrace:
				case Punctuation::Type::CloseBrace:
				case Punctuation::Type::Period:
				case Punctuation::Type::Comma: {
					evaluated.push_back(std::move(token));
					break;
				}
				case Punctuation::Type::Equals: {
					Token *next = tokens[i + 1].get();
					if (dynamic_cast<Punctuation *>(next)
					      && dynamic_cast<Punctuation *>(next)->type
					               == Punctuation::Type::Equals) {
						token->s.contents = "==";
						evaluated.push_back(std::make_unique<Operator>(*token,
						      Operator::Type::Equality));
						i++;
					} else {
						evaluated.push_back(std::make_unique<Operator>(*token,
						      Operator::Type::Assignment));
					}
					break;
				}
				case Punctuation::Type::Colon: {
					Token *next = tokens[i + 1].get();
					if (dynamic_cast<Punctuation *>(next)
					      && dynamic_cast<Punctuation *>(next)->type
					               == Punctuation::Type::Colon) {
						token->s.contents = "::";
						evaluated.push_back(
						      std::make_unique<Operator>(*token, Operator::Type::Scope));
						i++;
					} else {
						evaluated.push_back(std::move(token));
					}
					break;
				}
				case Punctuation::Type::Plus: {
					evaluated.push_back(
					      std::make_unique<Operator>(*token, Operator::Type::Addition));
					break;
				}
				case Punctuation::Type::Hyphen: {
					evaluated.push_back(std::make_unique<Operator>(*token,
					      Operator::Type::Subtraction));
					break;
				}
				case Punctuation::Type::Asterisk: {
					evaluated.push_back(std::make_unique<Operator>(*token,
					      Operator::Type::Multiplication));
					break;
				}
				case Punctuation::Type::ForwardSlash: {
					evaluated.push_back(
					      std::make_unique<Operator>(*token, Operator::Type::Division));
					break;
				}
				case Punctuation::Type::Percent: {
					evaluated.push_back(
					      std::make_unique<Operator>(*token, Operator::Type::Modulus));
					break;
				}
				case Punctuation::Type::Exclamation: {
					Token *next = tokens[i + 1].get();
					if (dynamic_cast<Punctuation *>(next)
					      && dynamic_cast<Punctuation *>(next)->type
					               == Punctuation::Type::Equals) {
						token->s.contents = "!=";
						evaluated.push_back(std::make_unique<Operator>(*token,
						      Operator::Type::Inequality));
						i++;
					} else {
						evaluated.push_back(std::make_unique<Operator>(*token,
						      Operator::Type::LogicalNot));
					}
					break;
				}
				case Punctuation::Type::LessThan: {
					Token *next = tokens[i + 1].get();
					if (dynamic_cast<Punctuation *>(next)
					      && dynamic_cast<Punctuation *>(next)->type
					               == Punctuation::Type::Equals) {
						token->s.contents = "<=";
						evaluated.push_back(std::make_unique<Operator>(*token,
						      Operator::Type::LessThanOrEqual));
						i++;
					} else if (dynamic_cast<Punctuation *>(next)
					           && dynamic_cast<Punctuation *>(next)->type
					                    == Punctuation::Type::LessThan) {
						token->s.contents = "<<";
						evaluated.push_back(std::make_unique<Operator>(*token,
						      Operator::Type::BitwiseShiftLeft));
						i++;
					} else {
						evaluated.push_back(std::make_unique<Operator>(*token,
						      Operator::Type::LessThan));
					}
					break;
				}
				case Punctuation::Type::GreaterThan: {
					Token *next = tokens[i + 1].get();
					if (dynamic_cast<Punctuation *>(next)
					      && dynamic_cast<Punctuation *>(next)->type
					               == Punctuation::Type::Equals) {
						token->s.contents = ">=";
						evaluated.push_back(std::make_unique<Operator>(*token,
						      Operator::Type::GreaterThanOrEqual));
						i++;
					} else if (dynamic_cast<Punctuation *>(next)
					           && dynamic_cast<Punctuation *>(next)->type
					                    == Punctuation::Type::GreaterThan) {
						token->s.contents = ">>";
						evaluated.push_back(std::make_unique<Operator>(*token,
						      Operator::Type::BitwiseShiftRight));
						i++;
					} else {
						evaluated.push_back(std::make_unique<Operator>(*token,
						      Operator::Type::GreaterThan));
					}
					break;
				}
				case Punctuation::Type::Ampersand: {
					Token *next = tokens[i + 1].get();
					if (dynamic_cast<Punctuation *>(next)
					      && dynamic_cast<Punctuation *>(next)->type
					               == Punctuation::Type::Ampersand) {
						token->s.contents = "&&";
						evaluated.push_back(std::make_unique<Operator>(*token,
						      Operator::Type::LogicalAnd));
						i++;
					} else {
						evaluated.push_back(std::make_unique<Operator>(*token,
						      Operator::Type::BitwiseAnd));
					}
					break;
				}
				case Punctuation::Type::VerticalBar: {
					Token *next = tokens[i + 1].get();
					if (dynamic_cast<Punctuation *>(next)
					      && dynamic_cast<Punctuation *>(next)->type
					               == Punctuation::Type::VerticalBar) {
						token->s.contents = "||";
						evaluated.push_back(std::make_unique<Operator>(*token,
						      Operator::Type::LogicalOr));
						i++;
					} else {
						evaluated.push_back(std::make_unique<Operator>(*token,
						      Operator::Type::BitwiseOr));
					}
					break;
				}
				case Punctuation::Type::Tilde: {
					evaluated.push_back(
					      std::make_unique<Operator>(*token, Operator::Type::BitwiseNot));
					break;
				}
				case Punctuation::Type::Caret: {
					evaluated.push_back(
					      std::make_unique<Operator>(*token, Operator::Type::BitwiseXor));
					break;
				}
			}
		} else if (dynamic_cast<SymbolOrLiteral *>(token.get())) {
			if (std::isdigit(dynamic_cast<SymbolOrLiteral *>(token.get())->s.contents[0])
			      != 0) {
				std::unique_ptr<Token> literal = evaluateIntegerLiteral(
				      dynamic_cast<SymbolOrLiteral *>(token.get()));
				if (literal == nullptr) {
					errorHandler->error(*token, "Invalid integer literal");
					evaluated.push_back(std::move(token));
				} else {
					evaluated.push_back(std::move(literal));
				}
			} else if (dynamic_cast<SymbolOrLiteral *>(token.get())->s.contents[0]
			           == '\'') {
				std::unique_ptr<Token> literal = evaluateCharacterLiteral(
				      dynamic_cast<SymbolOrLiteral *>(token.get()));
				if (literal == nullptr) {
					errorHandler->error(*token, "Invalid character literal");
					evaluated.push_back(std::move(token));
				} else {
					evaluated.push_back(std::move(literal));
				}
			} else if (dynamic_cast<SymbolOrLiteral *>(token.get())->s.contents
			           == "true") {
				evaluated.push_back(std::make_unique<BoolLiteral>(
				      *dynamic_cast<SymbolOrLiteral *>(token.get()), true));
			} else if (dynamic_cast<SymbolOrLiteral *>(token.get())->s.contents
			           == "false") {
				evaluated.push_back(std::make_unique<BoolLiteral>(
				      *dynamic_cast<SymbolOrLiteral *>(token.get()), false));
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

std::unique_ptr<IntegerLiteral> Lexer::evaluateIntegerLiteral(SymbolOrLiteral *literal) {
	int base = DECIMAL;
	size_t start = 0;
	size_t pos = 0;
	for (size_t i = 0; i < literal->s.contents.size(); i++) {
		if (!isDigitInBase(literal->s.contents[i], base)) {
			if (i == 1) {
				if (literal->s.contents[0] == '0') {
					if (literal->s.contents[1] == 'x') {
						base = HEXADECIMAL;
						start = 2;
						continue;
					}
					if (literal->s.contents[1] == 'o') {
						base = OCTAL;
						start = 2;
						continue;
					}
					if (literal->s.contents[1] == 'b') {
						base = BINARY;
						start = 2;
						continue;
					}
				}
			}
			std::string_view suffix = literal->s.contents.substr(i);
			std::string_view value = literal->s.contents.substr(start, i);
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
				return std::make_unique<IntegerLiteral>(*literal,
				      IntegerLiteral::Type::I8, intValue);
			}
			if (suffix == "i16") {
				uint64_t intValue = std::stoul(std::string(value), &pos, base);
				if (pos != i - start) {
					return nullptr;
				}
				return std::make_unique<IntegerLiteral>(*literal,
				      IntegerLiteral::Type::I16, intValue);
			}
			if (suffix == "i32") {
				uint64_t intValue = std::stoul(std::string(value), &pos, base);
				if (pos != i - start) {
					return nullptr;
				}
				return std::make_unique<IntegerLiteral>(*literal,
				      IntegerLiteral::Type::I32, intValue);
			}
			if (suffix == "i64") {
				uint64_t intValue = std::stoul(std::string(value), &pos, base);
				if (pos != i - start) {
					return nullptr;
				}
				return std::make_unique<IntegerLiteral>(*literal,
				      IntegerLiteral::Type::I64, intValue);
			}
			if (suffix == "u8") {
				uint64_t intValue = std::stoul(std::string(value), &pos, base);
				if (pos != i - start) {
					return nullptr;
				}
				return std::make_unique<IntegerLiteral>(*literal,
				      IntegerLiteral::Type::U8, intValue);
			}
			if (suffix == "u16") {
				uint64_t intValue = std::stoul(std::string(value), &pos, base);
				if (pos != i - start) {
					return nullptr;
				}
				return std::make_unique<IntegerLiteral>(*literal,
				      IntegerLiteral::Type::U16, intValue);
			}
			if (suffix == "u32") {
				uint64_t intValue = std::stoul(std::string(value), &pos, base);
				if (pos != i - start) {
					return nullptr;
				}
				return std::make_unique<IntegerLiteral>(*literal,
				      IntegerLiteral::Type::U32, intValue);
			}
			if (suffix == "u64") {
				uint64_t intValue = std::stoul(std::string(value), &pos, base);
				if (pos != i - start) {
					return nullptr;
				}
				return std::make_unique<IntegerLiteral>(*literal,
				      IntegerLiteral::Type::U64, intValue);
			}
			// Not a suffix, but not a digit, so not a valid literal
			return nullptr;
		}
	}
	// No non-digit character found, treat as regular i32 literal
	std::string_view value
	      = literal->s.contents.substr(start, literal->s.contents.size());
	uint64_t intValue = std::stoul(std::string(value), &pos, base);
	if (pos != literal->s.contents.size() - start) {
		return nullptr;
	}
	return std::make_unique<IntegerLiteral>(*literal, IntegerLiteral::Type::I32,
	      intValue);
}

std::unique_ptr<CharacterLiteral> Lexer::evaluateCharacterLiteral(
      SymbolOrLiteral *literal) {
	if (literal->s.contents.size() < 3) {
		return nullptr;
	}
	if (literal->s.contents[0] != '\'') {
		return nullptr;
	}
	if (literal->s.contents[1] == '\\') {
		if (literal->s.contents.size() < 4) {
			return nullptr;
		}
		if (literal->s.contents[3] != '\'') {
			return nullptr;
		}
		switch (literal->s.contents[2]) {
			case 'a':
				return std::make_unique<CharacterLiteral>(*literal, '\a');
			case 'b':
				return std::make_unique<CharacterLiteral>(*literal, '\b');
			case 'f':
				return std::make_unique<CharacterLiteral>(*literal, '\f');
			case 'n':
				return std::make_unique<CharacterLiteral>(*literal, '\n');
			case 'r':
				return std::make_unique<CharacterLiteral>(*literal, '\r');
			case 't':
				return std::make_unique<CharacterLiteral>(*literal, '\t');
			case 'v':
				return std::make_unique<CharacterLiteral>(*literal, '\v');
			case '\\':
				return std::make_unique<CharacterLiteral>(*literal, '\\');
			case '\'':
				return std::make_unique<CharacterLiteral>(*literal, '\'');
			case '\"':
				return std::make_unique<CharacterLiteral>(*literal, '\"');
			default:
				return nullptr;
		}
	}
	if (literal->s.contents[2] != '\'') {
		return nullptr;
	}
	char value = literal->s.contents[1];
	return std::make_unique<CharacterLiteral>(*literal, value);
}
