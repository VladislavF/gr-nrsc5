/* -*- c++ -*- */
/*
 * Copyright 2017 Clayton Smith.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "psd_encoder_impl.h"
#include <gnuradio/io_signature.h>
#include <boost/spirit/include/qi.hpp>

namespace gr {
namespace nrsc5 {

psd_encoder::sptr
psd_encoder::make(const int prog_num, const std::string& title, const std::string& artist)
{
    return gnuradio::get_initial_sptr(new psd_encoder_impl(prog_num, title, artist));
}


/*
 * The private constructor
 */
psd_encoder_impl::psd_encoder_impl(const int prog_num,
                                   const std::string& title,
                                   const std::string& artist)
    : gr::sync_block("psd_encoder",
                     gr::io_signature::make(0, 0, 0),
                     gr::io_signature::make(1, 1, sizeof(unsigned char)))
{
    this->prog_num = prog_num;
    this->title = title;
    this->artist = artist;
    seq_num = 0;
    packet_off = 0;

    message_port_register_in(pmt::mp("set_meta"));
    set_msg_handler(pmt::mp("set_meta"), [this](const pmt::pmt_t& msg) { this->set_meta(msg); });
}

/*
 * Our virtual destructor.
 */
psd_encoder_impl::~psd_encoder_impl() {}

void psd_encoder_impl::set_meta(const pmt::pmt_t& msg)
{
    using boost::spirit::qi::phrase_parse;
	using boost::spirit::qi::lexeme;
	using boost::spirit::qi::char_;
    using boost::spirit::qi::space;
	using boost::spirit::qi::blank;
    using boost::spirit::qi::lit;

    int msg_len = pmt::blob_length(pmt::cdr(msg));
	std::string in = std::string((char*)pmt::blob_data(pmt::cdr(msg)), msg_len);
	std::cout << "input string: " << in << "   length: " << in.size() << std::endl;

    std::string s1;

    //set the title
    if(phrase_parse(in.begin(), in.end(), "title" >> lexeme[+(char_ - '\n')] >> -lit("\n"),space, s1)) {
		std::cout << "Title: " << s1 << std::endl;
        title = s1;
    }
    //set the artist
    if(phrase_parse(in.begin(), in.end(), "artist" >> lexeme[+(char_ - '\n')] >> -lit("\n"),space, s1)) {
		std::cout << "Artist: " << s1 << std::endl;
        artist = s1;
    }
    std::cout << "Global Title: " << this->title << std::endl;
    std::cout << "Global Artist: " << this->artist << std::endl;
}

int psd_encoder_impl::work(int noutput_items,
                           gr_vector_const_void_star& input_items,
                           gr_vector_void_star& output_items)
{
    unsigned char* out = (unsigned char*)output_items[0];

    for (int off = 0; off < noutput_items; off++) {
        if (packet_off == packet.length()) {
            packet = encode_ppp(
                encode_psd_packet(BASIC_PACKET_FORMAT, PORT[prog_num], seq_num++));
            packet_off = 0;
        }
        out[off] = packet[packet_off++];
    }

    return noutput_items;
}

std::string psd_encoder_impl::encode_psd_packet(int dtpf, int port, int seq)
{
    std::string out;

    out += (char)(dtpf & 0xff);
    out += (char)(port & 0xff);
    out += (char)((port >> 8) & 0xff);
    out += (char)(seq & 0xff);
    out += (char)((seq >> 8) & 0xff);
    out += encode_id3();
    out += "UF";

    return out;
}

std::string psd_encoder_impl::encode_id3()
{
    std::string tit2("TIT2");
    std::string tpe1("TPE1");
    std::string payload = encode_frame(tit2, title) + encode_frame(tpe1, artist);
    int len = payload.length();
    std::string out;

    out += "ID3";
    out += (char)3;
    out += (char)0;
    out += (char)0;
    out += (char)((len >> 21) & 0x7f);
    out += (char)((len >> 14) & 0x7f);
    out += (char)((len >> 7) & 0x7f);
    out += (char)(len & 0x7f);
    out += payload;

    return out;
}

std::string psd_encoder_impl::encode_frame(std::string& id, std::string& data)
{
    int len = data.length() + 1;
    return id + (char)((len >> 24) & 0xff) + (char)((len >> 16) & 0xff) +
           (char)((len >> 8) & 0xff) + (char)(len & 0xff) + (char)0 + (char)0 + (char)0 +
           data;
}

std::string psd_encoder_impl::encode_ppp(std::string packet)
{
    int fcs = compute_fcs(packet);
    packet += (char)(fcs & 0xff);
    packet += (char)(fcs >> 8);
    std::string out;

    out += (char)0x7e;
    for (int i = 0; i < packet.length(); i++) {
        char byte = packet[i];
        switch (byte) {
        case 0x7d:
        case 0x7e:
            out += (char)0x7d;
            out += (char)(byte ^ 0x20);
            break;
        default:
            out += byte;
        }
    }

    return out;
}

int psd_encoder_impl::compute_fcs(std::string& packet)
{
    int fcs = 0xffff;
    for (int i = 0; i < packet.length(); i++) {
        fcs = (fcs >> 8) ^ FCS_TABLE[(fcs ^ packet[i]) & 0xff];
    }
    return fcs ^ 0xffff;
}

} /* namespace nrsc5 */
} /* namespace gr */
