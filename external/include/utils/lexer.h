/*
================================================================================================

Description	:	Basic C99-style lexer.
Author		:	J.M.P. van Waveren
Date		:	12/10/2016
Language	:	C99
Format		:	Real tabs with the tab size equal to 4 spaces.
Copyright	:	Copyright (c) 2016 Oculus VR, LLC. All Rights reserved.


LICENSE
=======

Copyright (c) 2016 Oculus VR, LLC.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

DESCRIPTION
===========

Basic C99-style lexer.

This is a zero-allocation stateless lexer. A token is returned as a pointer in the original input buffer.
As a result:
- All tokens are left untouched, including escape sequencies in strings and literals.
- Multi-line strings are not automatically merged into a single token.


INTERFACE
=========

ksTokenType
ksTokenFlags
ksTokenInfo

static const unsigned char * ksLexer_NextToken( const unsigned char * buffer, const unsigned char * ptr, const unsigned char ** token, ksTokenInfo * tokenInfo );
static const unsigned char * ksLexer_SkipUpToIncludingToken( const unsigned char * buffer, const unsigned char * ptr, const char * token );
static const unsigned char * ksLexer_SkipUpToEndOfLine( const unsigned char * buffer, const unsigned char * ptr );
static const unsigned char * ksLexer_SkipBracedSection( const unsigned char * buffer, const unsigned char * ptr );
static bool ksLexer_CaseSensitiveCompareToken( const unsigned char * tokenStart, const unsigned char * tokenEnd, const char * value );

=================================================================================
*/

#if !defined( KSLEXER_H )
#define KSLEXER_H

typedef enum
{
	KS_TOKEN_TYPE_NONE,
	KS_TOKEN_TYPE_NAME,
	KS_TOKEN_TYPE_STRING,
	KS_TOKEN_TYPE_LITERAL,
	KS_TOKEN_TYPE_NUMBER,
	KS_TOKEN_TYPE_PUNCTUATION
} ksTokenType;

typedef enum
{
	KS_TOKEN_FLAG_NONE			= 0,
	KS_TOKEN_FLAG_DECIMAL		= BIT( 0 ),
	KS_TOKEN_FLAG_OCTAL			= BIT( 1 ),
	KS_TOKEN_FLAG_HEXADECIMAL	= BIT( 2 ),
	KS_TOKEN_FLAG_UNSIGNED		= BIT( 3 ),
	KS_TOKEN_FLAG_LONG			= BIT( 4 ),
	KS_TOKEN_FLAG_LONG_LONG		= BIT( 5 ),
	KS_TOKEN_FLAG_FLOAT			= BIT( 6 ),
	KS_TOKEN_FLAG_DOUBLE		= BIT( 7 )
} ksTokenFlags;

typedef struct ksTokenInfo
{
	ksTokenType			type;			// Token type.
	ksTokenFlags		flags;			// Token flags.
	int					linesCrossed;	// Number of lines crossed before the token.
} ksTokenInfo;

// Gets the next C99-style token from a zero-terminated buffer.
// 'buffer' is the base pointer of the buffer and 'ptr' is the current pointer into the buffer.
// A pointer to the next token is returned in 'token' and if 'tokenInfo' is not NULL, then additional information is returned in 'tokenInfo'.
// Returns a pointer to the first character after the token.
// The length of a token is the returned pointer minus the token pointer stored in 'token'.
static const unsigned char * ksLexer_NextToken( const unsigned char * buffer, const unsigned char * ptr, const unsigned char ** token, ksTokenInfo * tokenInfo )
{
	const unsigned char * start = ptr;
	int linesCrossed = 0;

	// Parse non-tokens.
	while ( ptr[0] != '\0' )
	{
		// Parse white space
		while ( ptr[0] != '\0' && ptr[0] <= ' ' )
		{
			linesCrossed += ( ptr[0] == '\n' );
			ptr++;
		}
		// Parse comment.
		if ( ptr[0] == '/' )
		{
			if ( ptr[1] == '/' )
			{
				ptr += 2;
				while ( ptr[0] != '\0' && ptr[0] != '\n' )
				{
					ptr++;
				}
				continue;
			}
			else if ( ptr[1] == '*' )
			{
				ptr += 2;
				while ( ptr[0] != '\0' && ( ptr[0] != '*' || ptr[1] != '/' ) )
				{
					linesCrossed += ( ptr[0] == '\n' );
					ptr++;
				}
				ptr += 2 * ( ptr[0] != '\0' );
				continue;
			}
		}
		break;
	}

	// Save off pointer to token.
	*token = ptr;

	// Parse name token.
	{
		while (	( ptr[0] >= 'a' && ptr[0] <= 'z' ) ||
				( ptr[0] >= 'A' && ptr[0] <= 'Z' ) ||
				( ptr[0] == '_' ) ||
				( ( ptr[0] >= '0' && ptr[0] <= '9' ) && ptr > *token ) )
		{
			ptr++;
		}
		if ( ptr > *token )
		{
			if ( tokenInfo != NULL )
			{
				tokenInfo->type = KS_TOKEN_TYPE_NAME;
				tokenInfo->flags = KS_TOKEN_FLAG_NONE;
				tokenInfo->linesCrossed = linesCrossed;
			}
			return ptr;
		}
	}

	// Parse string or literal token.
	if ( ptr[0] == '\"' || ptr[0] == '\'' )
	{
		const char firstChar = ptr[0];
		ptr++;
		while ( ptr[0] != '\0' && ptr[0] != firstChar )
		{
			assert( ptr[0] != '\n' );

			if ( ptr[0] == '\\' )
			{
				ptr++;
				if ( ptr[0] == 'x' || ptr[0] == 'X' || ptr[0] == 'u' || ptr[0] == 'U' )
				{
					// Parse hexadecimal or Unicode.
					while ( ( ptr[0] >= '0' && ptr[0] <= '9' ) ||
							( ptr[0] >= 'a' && ptr[0] <= 'f' ) ||
							( ptr[0] >= 'A' && ptr[0] <= 'F' ) )
					{
						ptr++;
					}
				}
				else if ( ptr[0] >= '0' && ptr[0] <= '7' )
				{
					// Parse octal.
					do
					{
						ptr++;
					}
					while ( ptr[0] >= '0' && ptr[0] <= '7' );
				}
				else
				{
					ptr++;
				}
			}
			else
			{
				ptr++;
			}
		}
		ptr++;
		if ( tokenInfo != NULL )
		{
			tokenInfo->type = ( firstChar == '\"' ) ? KS_TOKEN_TYPE_STRING : KS_TOKEN_TYPE_LITERAL;
			tokenInfo->flags = KS_TOKEN_FLAG_NONE;
			tokenInfo->linesCrossed = linesCrossed;
		}
		return ptr;
	}

	// Parse number token.
	{
		// Parse sign.
		if (	( ptr[0] == '+' || ptr[0] == '-' )
				&&
				( ( ptr[1] >= '0' && ptr[1] <= '9' ) || ( ptr[1] == '.' ) )
				&&
				(
					( start == buffer )
					||
					(
						!( start[-1] >= 'a' && start[-1] <= 'z' ) &&
						!( start[-1] >= 'A' && start[-1] <= 'Z' ) &&
						!( start[-1] == '_' || start[-1] == ')' || start[1] == ']' ) &&
						!( start[-1] >= '0' && start[-1] <= '9' )
					)
				)
			)
		{
			ptr++;
		}

		ksTokenFlags flags = KS_TOKEN_FLAG_DECIMAL;

		// Parse octal or hexadecimal.
		if ( ptr[0] == '0' )
		{
			if ( ptr[1] == 'x' || ptr[1] == 'X' )
			{
				ptr += 2;
				while ( ( ptr[0] >= '0' && ptr[0] <= '9' ) ||
						( ptr[0] >= 'a' && ptr[0] <= 'f' ) ||
						( ptr[0] >= 'A' && ptr[0] <= 'F' ) )
				{
					ptr++;
				}
				flags = KS_TOKEN_FLAG_HEXADECIMAL;
			}
			else if ( ptr[1] >= '0' && ptr[1] <= '7' )
			{
				ptr += 2;
				while ( ptr[0] >= '0' && ptr[0] <= '7' )
				{
					ptr++;
				}
				flags = KS_TOKEN_FLAG_OCTAL;
			}
		}

		// Parse decimal integer or floating-point.
		if ( flags == KS_TOKEN_FLAG_DECIMAL )
		{
			for ( bool hasDigit = false; ptr[0] != '\0'; )
			{
				if ( ptr[0] >= '0' && ptr[0] <= '9' )
				{
					ptr++;
					hasDigit = true;
					continue;
				}
				if ( ptr[0] == '.' &&
						( hasDigit || ( ptr[1] >= '0' && ptr[1] <= '9' ) ) )
				{
					ptr++;
					flags |= KS_TOKEN_FLAG_DOUBLE;
					continue;
				}
				if ( hasDigit )
				{
					if (	( ptr[0] == 'e' || ptr[0] == 'E' )
							&&
							(
								( ptr[1] >= '0' && ptr[1] <= '9' )
								||
								(
									( ptr[1] == '+' || ptr[1] == '-' )
									&&
									( ptr[2] >= '0' && ptr[2] <= '9' )
								)
							)
						)
					{
						ptr++;
						if ( ptr[0] == '+' || ptr[0] == '-' )
						{
							ptr++;
						}
						flags |= KS_TOKEN_FLAG_DOUBLE;
						continue;
					}
					if ( ptr[0] == 'f' || ptr[0] == 'F' )
					{
						flags &= ~KS_TOKEN_FLAG_DOUBLE;
						flags |= KS_TOKEN_FLAG_FLOAT;
						ptr++;
						break;
					}
				}
				break;
			}
		}

		// Identify unsigned, long and long long integers.
		if ( ( flags & ( KS_TOKEN_FLAG_FLOAT | KS_TOKEN_FLAG_DOUBLE ) ) == 0 )
		{
			while ( ptr[0] != '\0' )
			{
				if ( ptr[0] == 'u' || ptr[0] == 'U' )
				{
					flags |= KS_TOKEN_FLAG_UNSIGNED;
					ptr++;
				}
				else if ( ptr[0] == 'l' || ptr[0] == 'L' )
				{
					if ( ptr[1] == 'l' || ptr[1] == 'L' )
					{
						flags |= KS_TOKEN_FLAG_LONG_LONG;
						ptr += 2;
					}
					else
					{
						flags |= KS_TOKEN_FLAG_LONG;
						ptr++;
					}
				}
				else
				{
					break;
				}
			}
		}

		if ( ptr > *token )
		{
			if ( tokenInfo != NULL )
			{
				tokenInfo->type = KS_TOKEN_TYPE_NUMBER;
				tokenInfo->flags = flags;
				tokenInfo->linesCrossed = linesCrossed;
			}
			return ptr;
		}
	}

	// Parse punctuation token.
	switch ( ptr[0] )
	{
		// Handle multi-character operators.
		case ':': ptr++; ptr += ( ptr[0] == ':' ); break;										// ::
		case '+': ptr++; ptr += ( ptr[0] == '+' || ptr[0] == '=' ); break;						// ++ +=
		case '-': ptr++; ptr += ( ptr[0] == '-' || ptr[0] == '=' || ptr[0] == '>' ); break;		// -- -= ->
		case '*': ptr++; ptr += ( ptr[0] == '=' ); break;										// *=
		case '/': ptr++; ptr += ( ptr[0] == '=' ); break;										// /=
		case '%': ptr++; ptr += ( ptr[0] == '=' ); break;										// %=
		case '<': ptr++; ptr += ( ptr[0] == '<' ); ptr += ( ptr[0] == '=' ); break;				// << <= <<=
		case '>': ptr++; ptr += ( ptr[0] == '>' ); ptr += ( ptr[0] == '=' ); break;				// >> >= >>=
		case '=': ptr++; ptr += ( ptr[0] == '=' ); break;										// ==
		case '!': ptr++; ptr += ( ptr[0] == '=' ); break;										// !=
		case '^': ptr++; ptr += ( ptr[0] == '=' ); break;										// ^=
		case '&': ptr++; ptr += ( ptr[0] == '=' || ptr[0] == '&' ); break;						// &= &&
		case '|': ptr++; ptr += ( ptr[0] == '=' || ptr[0] == '|' ); break;						// |= ||
		default:
		{
			// Consider any non-character and non-digit a punctuation,
			// including C-operators: ( ) [ ] . ? , and @ # $ ` ;
			if (	ptr[0] > ' ' &&
					!( ptr[0] >= 'a' && ptr[0] <= 'z' ) &&
					!( ptr[0] >= 'A' && ptr[0] <= 'Z' ) &&
					!( ptr[0] == '_' ) &&
					!( ptr[0] >= '0' && ptr[0] <= '9' ) )
			{
				ptr++;
			}
		}
	}
	if ( tokenInfo != NULL )
	{
		tokenInfo->type = ( ptr > *token ) ? KS_TOKEN_TYPE_PUNCTUATION : KS_TOKEN_TYPE_NONE;
		tokenInfo->flags = KS_TOKEN_FLAG_NONE;
		tokenInfo->linesCrossed = linesCrossed;
	}
	return ptr;
}

// Case-sensitive compare the non-zero terminated token to the given zero-terminated value.
// If 'tokenEnd' is NULL then 'tokenStart' is expected to be zero terminated.
static bool ksLexer_CaseSensitiveCompareToken( const unsigned char * tokenStart, const unsigned char * tokenEnd, const char * value )
{
	if ( value == NULL )
	{
		return false;
	}
	if ( tokenEnd == NULL )
	{
		return ( strcmp( (const char *)tokenStart, value ) == 0 );
	}
	const size_t length = strlen( value );
	return ( (size_t)( tokenEnd - tokenStart ) == length && strncmp( (const char *)tokenStart, value, length ) == 0 );
}

// Skip up to and including the given token.
// Returns a pointer to the first character after the given token.
static const unsigned char * ksLexer_SkipUpToIncludingToken( const unsigned char * buffer, const unsigned char * ptr, const char * token )
{
	while ( ptr[0] != '\0' )
	{
		const unsigned char * t;
		ptr = ksLexer_NextToken( buffer, ptr, &t, NULL );
		if ( ksLexer_CaseSensitiveCompareToken( t, ptr, token ) )
		{
			break;
		}
	}
	return ptr;
}

// Skip up to the end of the line.
// Returns a pointer to the first character after the token after which a line is crossed.
static const unsigned char * ksLexer_SkipUpToEndOfLine( const unsigned char * buffer, const unsigned char * ptr )
{
	while ( ptr[0] != '\0' )
	{
		const unsigned char * token;
		ksTokenInfo info;
		const unsigned char * newPtr = ksLexer_NextToken( buffer, ptr, &token, &info );
		if ( info.linesCrossed > 0 )
		{
			break;
		}
		ptr = newPtr;
	}
	return ptr;
}

// Skip the next curly braced section including any nested curly braced sections.
// The opening curly brace is expected to appear after the 'ptr' in the input 'buffer'.
// Any tokens before the first opening curly brace are skipped as well.
// Returns a pointer to the first character after the closing curly brace.
static const unsigned char * ksLexer_SkipBracedSection( const unsigned char * buffer, const unsigned char * ptr )
{
	int braceDepth = 0;
	while ( ptr[0] != '\0' )
	{
		const unsigned char * token;
		ptr = ksLexer_NextToken( buffer, ptr, &token, NULL );
		if ( token[0] == '{' )
		{
			braceDepth++;
		}
		else if ( token[0] == '}' )
		{
			braceDepth--;
			if ( braceDepth == 0 )
			{
				break;
			}
		}
	}
	return ptr;
}

#endif // !KSLEXER_H
