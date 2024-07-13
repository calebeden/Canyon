#ifndef TEST_UTILITIES_H
#define TEST_UTILITIES_H

#include "errorhandler.h"

#include "gtest/gtest.h"

#include <filesystem>
#include <iostream>
#include <queue>
#include <string>
#include <tuple>

class NoErrorHandler : public ErrorHandler {
public:
	NoErrorHandler() = default;
	NoErrorHandler(const NoErrorHandler &other) = delete;
	NoErrorHandler &operator=(const NoErrorHandler &other) = delete;
	NoErrorHandler(NoErrorHandler &&other) = delete;
	NoErrorHandler &operator=(NoErrorHandler &&other) = delete;

	bool handleErrors([[maybe_unused]] std::ostream &os) override {
		EXPECT_TRUE(errors.empty());
		return false;
	}

	~NoErrorHandler() override {
		EXPECT_TRUE(errors.empty());
	}
};

class HasErrorHandler : public ErrorHandler {
private:
	bool checked = false;
public:
	HasErrorHandler() = default;
	HasErrorHandler(const HasErrorHandler &other) = delete;
	HasErrorHandler &operator=(const HasErrorHandler &other) = delete;
	HasErrorHandler(HasErrorHandler &&other) = delete;
	HasErrorHandler &operator=(HasErrorHandler &&other) = delete;

	bool handleErrors([[maybe_unused]] std::ostream &os) override {
		ADD_FAILURE();
		return false;
	}

	void checkErrors(
	      std::queue<std::tuple<std::filesystem::path, size_t, size_t, std::string>>
	            expected) {
		ASSERT_FALSE(checked) << "Errors already checked";
		EXPECT_EQ(errors.size(), expected.size());
		for (; !errors.empty(); errors.pop(), expected.pop()) {
			const auto &actual = errors.front();
			const auto &expect = expected.front();
			EXPECT_EQ(actual->source, std::get<0>(expect));
			EXPECT_EQ(dynamic_cast<ErrorWithLocation *>(actual.get())->row,
			      std::get<1>(expect));
			EXPECT_EQ(dynamic_cast<ErrorWithLocation *>(actual.get())->col,
			      std::get<2>(expect));
			EXPECT_EQ(actual->message, std::get<3>(expect));
		}
		checked = true;
	}

	~HasErrorHandler() override {
		EXPECT_TRUE(checked) << "Errors not checked";
	}
};

#endif
